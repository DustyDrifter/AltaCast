#ifndef _config_io_callback_h_
#define _config_io_callback_h_

class NOVTABLE config_io_callback : public service_base
{
public:
	virtual void on_read() = 0;
	virtual void on_write(bool reset) = 0;

	//for core use only
	static void g_on_read();
	static void g_on_write(bool reset);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(config_io_callback);
};

#endif //_config_io_callback_h_