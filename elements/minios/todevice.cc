#include "todevice.hh"

#include <click/args.hh>
#include <click/dequeue.hh>
#include <click/error.hh>
#include <click/router.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/task.hh>
#include <stdio.h>

extern "C" {
#include <netfront.h>
}


CLICK_DECLS

ToDevice::ToDevice()
	: _vifid(0), _burstsize(1), _count(0), _task(this), _dev(NULL)
{
}

ToDevice::~ToDevice()
{
}

int
ToDevice::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (Args(conf, this, errh)
			.read_p("DEVID", IntArg(), _vifid)
			.read_p("BURST", IntArg(), _burstsize)
			.complete() < 0)
		return -1;

	if (_vifid < 0)
		return errh->error("Interface id must be >= 0");

    if (_burstsize < 1)
        return errh->error("Burst size must be >= 1");

	return 0;
}

int
ToDevice::initialize(ErrorHandler *errh)
{
	char nodename[256];
	snprintf(nodename, sizeof(nodename), "device/vif/%d", _vifid);

	_dev = init_netfront(nodename, NULL, NULL, NULL);
	if (!_dev)
		return errh->error("Unable to initialize netfront for device %d (%s)", _vifid, nodename);

    /* TODO: should probably move this to configure */
	void *&used = router()->force_attachment("device_writer_" + String(nodename));
	if (used)
		return errh->error("Duplicate writer for device `%d'", _vifid);

	used = this;

	if (input_is_pull(0)) {
		ScheduleInfo::initialize_task(this, &_task, errh);
	}

	return 0;
}

void
ToDevice::cleanup(CleanupStage stage)
{
	shutdown_netfront(_dev);
	_dev = NULL;
}

void
ToDevice::push(int port, Packet *p)
{
    netfront_xmit(_dev, (unsigned char*) p->data(), p->length());
    checked_output_push(0, p);
}

bool
ToDevice::run_task(Task *)
{
	int c;
	Packet* p;

	for (c = 0; c < _burstsize; c++) {
		p = input(0).pull();
		if (!p)
			break;

		netfront_xmit(_dev, (unsigned char*) p->data(), p->length());
		checked_output_push(0, p);
	}

	/* TODO: should only fast_reschedule when there is more work to do? */
	_task.fast_reschedule();

	return c > 0;
}

void
ToDevice::add_handlers()
{
    add_read_handler("count", read_handler, 0);
    add_write_handler("reset_counts", reset_counts, 0, Handler::BUTTON);
}

String
ToDevice::read_handler(Element* e, void *thunk)
{
    return String(static_cast<ToDevice*>(e)->_count);
}

int
ToDevice::reset_counts(const String &, Element *e, void *, ErrorHandler *)
{
    static_cast<ToDevice*>(e)->_count = 0;
    return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ToDevice)
