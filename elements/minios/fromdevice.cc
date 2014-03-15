#include "fromdevice.hh"

#include <click/args.hh>
#include <click/dequeue.hh>
#include <click/error.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/task.hh>

#include <netfront.h>


CLICK_DECLS

FromDevice::FromDevice()
	: _vifid(0), _count(0), _task(this), _dev(NULL)
{
}

FromDevice::~FromDevice()
{
}

int
FromDevice::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (Args(conf, this, errh)
			.read_p("DEVID", IntArg(), _vifid)
			.complete() < 0)
		return -1;

	if (_vifid < 0)
		return errh->error("Interface id must be >= 0");

	return 0;
}

int
FromDevice::initialize(ErrorHandler *errh)
{
	char nodename[256];
	snprintf(nodename, sizeof(nodename), "device/vif/%d", _vifid);

	_dev = init_netfront(nodename, NULL, NULL, NULL);
	if (!_dev)
		return errh->error("Unable to initialize netfront for device %d (%s)", _vifid, nodename);

	netfront_set_rx_handler(_dev, FromDevice::rx_handler, (void*)this);

	ScheduleInfo::initialize_task(this, &_task, errh);

	return 0;
}

void
FromDevice::cleanup(CleanupStage stage)
{
	shutdown_netfront(_dev);
	_dev = NULL;
}

bool
FromDevice::run_task(Task *)
{
	int c;

	network_rx(_dev);
	c = _dequeue.size();

	for (int i = 0; likely(i < c); i++) {
		output(0).push(_dequeue.front());
		_dequeue.pop_front();
	}
	_count += c;

	_task.fast_reschedule();

	return c > 0;
}

void
FromDevice::rx_handler(unsigned char* data, int len, void* e)
{
	Packet* p = Packet::make(data, len, FromDevice::pkt_destructor);
	((FromDevice*) e)->_dequeue.push_back(p);
}

void
FromDevice::add_handlers()
{
    add_read_handler("count", read_handler, 0);
    add_write_handler("reset_counts", reset_counts, 0, Handler::BUTTON);
}

String
FromDevice::read_handler(Element* e, void *thunk)
{
    return String(static_cast<FromDevice*>(e)->_count);
}

int
FromDevice::reset_counts(const String &, Element *e, void *, ErrorHandler *)
{
    static_cast<FromDevice*>(e)->_count = 0;
    return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(FromDevice)
