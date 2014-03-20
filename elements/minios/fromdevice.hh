#ifndef CLICK_FROMDEVICE_HH
#define CLICK_FROMDEVICE_HH

#include <click/config.h>
#include <click/dequeue.hh>
#include <click/element.hh>
#include <click/error.hh>
#include <click/task.hh>

extern "C" {
#include <netfront.h>
}


CLICK_DECLS

class FromDevice : public Element {
public:
    FromDevice();
    ~FromDevice();

    const char *class_name() const { return "FromDevice"; }
    const char *port_count() const { return "0/1"; }
    const char *processing() const { return "/h"; }
    int configure_phase() const { return CONFIGURE_PHASE_FIRST; }

    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);

    void add_handlers();

    bool run_task(Task *);

private:
    int _vifid;
    int _count;
    Task _task;
    DEQueue<Packet*> _dequeue;
    struct netfront_dev* _dev;

    static void rx_handler(unsigned char* data, int len, void* e);
    static void pkt_destructor(unsigned char* data, size_t lenght) {};

    static String read_handler(Element* e, void *thunk);
    static int reset_counts(const String &, Element *e, void *, ErrorHandler *);
};

CLICK_ENDDECLS
#endif
