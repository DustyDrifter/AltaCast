#include "pfc.h"



/*
class NOVTABLE bsearch_callback
{
public:
	virtual int test(t_size p_index) const = 0;
};
*/

PFC_DLL_EXPORT bool pfc::bsearch(t_size p_count, pfc::bsearch_callback const & p_callback,t_size & p_result)
{
	return bsearch_inline_t(p_count,p_callback,p_result);
}