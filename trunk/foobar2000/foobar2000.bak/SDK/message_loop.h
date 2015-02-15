class NOVTABLE message_filter
{
public:
    virtual bool pretranslate_message(MSG * p_msg) = 0;
};
 
class NOVTABLE idle_handler
{
public:
    virtual bool on_idle() = 0;
};
 
class NOVTABLE message_loop : public service_base
{
public:
    virtual void add_message_filter(message_filter * ptr) = 0;
    virtual void remove_message_filter(message_filter * ptr) = 0;
 
    virtual void add_idle_handler(idle_handler * ptr) = 0;
    virtual void remove_idle_handler(idle_handler * ptr) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(message_loop);
};