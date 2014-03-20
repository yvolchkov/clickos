#ifndef CLICK_TODEVICE_HH
#define CLICK_TODEVICE_HH

#include <click/config.h>
#include <click/element.hh>
#include <click/error.hh>
#include <click/task.hh>

extern "C" {
#include <netfront.h>
}


CLICK_DECLS

class ToDevice : public Element {
public:
    ToDevice();
    ~ToDevice();

    const char *class_name() const { return "ToDevice"; }
    const char *port_count() const { return "1/0-1"; }
    const char *processing() const { return "a/h"; }
    int configure_phase() const { return CONFIGURE_PHASE_FIRST; }

    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);

    void add_handlers();

    bool run_task(Task *);
    void push(int, Packet *p);

private:
    int _vifid;
    int _burstsize;
    int _count;
    Task _task;
    struct netfront_dev* _dev;

    static String read_handler(Element* e, void *thunk);
    static int reset_counts(const String &, Element *e, void *, ErrorHandler *);
};

CLICK_ENDDECLS
#endif
