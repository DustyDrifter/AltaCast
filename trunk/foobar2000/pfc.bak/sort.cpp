#include "pfc.h"

namespace pfc {

PFC_DLL_EXPORT void swap_void(void * item1,void * item2,t_size width)
{
	unsigned char * ptr1 = (unsigned char*)item1, * ptr2 = (unsigned char*)item2;
	t_size n;
	unsigned char temp;
	for(n=0;n<width;n++)
	{
		temp = *ptr2;
		*ptr2 = *ptr1;
		*ptr1 = temp;
		ptr1++;
		ptr2++;
	}
}

PFC_DLL_EXPORT void reorder(reorder_callback & p_callback,const t_size * p_order,t_size p_count)
{
	t_size done_size = bit_array_bittable::g_estimate_size(p_count);
	pfc::array_hybrid_t<unsigned char,1024> done;
	done.set_size(done_size);
	pfc::memset_t(done,(unsigned char)0);
	t_size n;
	for(n=0;n<p_count;n++)
	{
		t_size next = p_order[n];
		if (next!=n && !bit_array_bittable::g_get(done,n))
		{
			t_size prev = n;
			do
			{
				assert(!bit_array_bittable::g_get(done,next));
				assert(next>n);
				assert(n<p_count);
				p_callback.swap(prev,next);
				bit_array_bittable::g_set(done,next,true);
				prev = next;
				next = p_order[next];
			} while(next!=n);
			//bit_array_bittable::g_set(done,n,true);
		}
	}
}

PFC_DLL_EXPORT void reorder_void(void * data,t_size width,const t_size * order,t_size num,void (*swapfunc)(void * item1,void * item2,t_size width))
{
	unsigned char * base = (unsigned char *) data;
	t_size done_size = bit_array_bittable::g_estimate_size(num);
	pfc::array_hybrid_t<unsigned char,1024> done;
	done.set_size(done_size);
	pfc::memset_t(done,(unsigned char)0);
	t_size n;
	for(n=0;n<num;n++)
	{
		t_size next = order[n];
		if (next!=n && !bit_array_bittable::g_get(done,n))
		{
			t_size prev = n;
			do
			{
				assert(!bit_array_bittable::g_get(done,next));
				assert(next>n);
				assert(n<num);
				swapfunc(base+width*prev,base+width*next,width);
				bit_array_bittable::g_set(done,next,true);
				prev = next;
				next = order[next];
			} while(next!=n);
			//bit_array_bittable::g_set(done,n,true);
		}
	}
}

namespace {

class sort_callback_impl_legacy : public sort_callback
{
public:
	sort_callback_impl_legacy(
		void * p_base,t_size p_width, 
		int (*p_comp)(const void *, const void *),
		void (*p_swap)(void *, void *, t_size)
		) :
	m_base((char*)p_base), m_width(p_width),
	m_comp(p_comp), m_swap(p_swap)
	{
	}


	int compare(t_size p_index1, t_size p_index2) const
	{
		return m_comp(m_base + p_index1 * m_width, m_base + p_index2 * m_width);
	}
	
	void swap(t_size p_index1, t_size p_index2)
	{
		m_swap(m_base + p_index1 * m_width, m_base + p_index2 * m_width, m_width);
	}

private:
	char * m_base;
	t_size m_width;
    int (*m_comp)(const void *, const void *);
	void (*m_swap)(void *, void *, t_size);
};
}

PFC_DLL_EXPORT void sort_void_ex (
    void *base,
    t_size num,
    t_size width,
    int (*comp)(const void *, const void *),
	void (*swap)(void *, void *, t_size) )
{
	sort(sort_callback_impl_legacy(base,width,comp,swap),num);
}



/***
*qsort.c - quicksort algorithm; qsort() library function for sorting arrays
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       To implement the qsort() routine for sorting arrays.
*
*******************************************************************************/



/* prototypes for local routines */
static void shortsort(t_size lo, t_size hi, sort_callback & p_callback);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */

/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*       maximum array size is number of elements times size of elements,
*       but is limited by the virtual address space of the processor
*
*Entry:
*       char *base = pointer to base of array
*       size_t num  = number of elements in the array
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

#define STKSIZ (8*sizeof(void*) - 2)

PFC_DLL_EXPORT void sort(sort_callback & p_callback,t_size num)
{
    /* Note: the number of stack entries required is no more than
       1 + log2(num), so 30 is sufficient for any array */
    t_size lo, hi;              /* ends of sub-array currently sorting */
    t_size mid;                  /* points to middle of subarray */
    t_size loguy, higuy;        /* traveling pointers for partition step */
    t_size size;                /* size of the sub-array */
    t_size lostk[STKSIZ], histk[STKSIZ];
    int stkptr;                 /* stack for saving sub-array to be processed */

	enum {base = 0, width = 1};
    if (num < 2)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = base;
    hi = base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       preserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
        shortsort(lo, hi, p_callback);
    }
    else {
        /* First we pick a partitioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the median
           of the values, but also that we select one fast.  We choose the
           median of the first, middle, and last elements, to avoid bad
           performance in the face of already sorted data, or data that is made
           up of multiple sorted runs appended together.  Testing shows that a
           median-of-three algorithm provides better performance than simply
           picking the middle element for the latter case. */

        mid = lo + (size / 2) * width;      /* find middle element */

        /* Sort the first, middle, last elements into order */
        if (p_callback.compare(lo, mid) > 0) {
            p_callback.swap(lo, mid);
        }
        if (p_callback.compare(lo, hi) > 0) {
            p_callback.swap(lo, hi);
        }
        if (p_callback.compare(mid, hi) > 0) {
            p_callback.swap(mid, hi);
        }

        /* We now wish to partition the array into three pieces, one consisting
           of elements <= partition element, one of elements equal to the
           partition element, and one of elements > than it.  This is done
           below; comments indicate conditions established at every step. */

        loguy = lo;
        higuy = hi;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi,
               A[i] <= A[mid] for lo <= i <= loguy,
               A[i] > A[mid] for higuy <= i < hi,
               A[hi] >= A[mid] */

            /* The doubled loop is to avoid calling comp(mid,mid), since some
               existing comparison funcs don't work when passed the same
               value for both pointers. */

            if (mid > loguy) {
                do  {
                    loguy += width;
                } while (loguy < mid && p_callback.compare(loguy, mid) <= 0);
            }
            if (mid <= loguy) {
                do  {
                    loguy += width;
                } while (loguy <= hi && p_callback.compare(loguy, mid) <= 0);
            }

            /* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[mid] */

            do  {
                higuy -= width;
            } while (higuy > mid && p_callback.compare(higuy, mid) > 0);

            /* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
               either higuy == lo or A[higuy] <= A[mid] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy == lo, then we would have exited, so
               A[loguy] > A[mid], A[higuy] <= A[mid],
               loguy <= hi, higuy > lo */

            p_callback.swap(loguy, higuy);

            /* If the partition element was moved, follow it.  Only need
               to check for mid == higuy, since before the swap,
               A[loguy] > A[mid] implies loguy != mid. */

            if (mid == higuy)
                mid = loguy;

            /* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
               of loop is re-established */
        }

        /*     A[i] <= A[mid] for lo <= i < loguy,
               A[i] > A[mid] for higuy < i < hi,
               A[hi] >= A[mid]
               higuy < loguy
           implying:
               higuy == loguy-1
               or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */

        /* Find adjacent elements equal to the partition element.  The
           doubled loop is to avoid calling comp(mid,mid), since some
           existing comparison funcs don't work when passed the same value
           for both pointers. */

        higuy += width;
        if (mid < higuy) {
            do  {
                higuy -= width;
            } while (higuy > mid && p_callback.compare(higuy, mid) == 0);
        }
        if (mid >= higuy) {
            do  {
                higuy -= width;
            } while (higuy > lo && p_callback.compare(higuy, mid) == 0);
        }

        /* OK, now we have the following:
              higuy < loguy
              lo <= higuy <= hi
              A[i]  <= A[mid] for lo <= i <= higuy
              A[i]  == A[mid] for higuy < i < loguy
              A[i]  >  A[mid] for loguy <= i < hi
              A[hi] >= A[mid] */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - lo >= hi - loguy ) {
            if (lo < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo < higuy) {
                hi = higuy;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void shortsort(t_size lo, t_size hi, sort_callback & p_callback)
{
    t_size p, max;
	enum { width = 1 } ;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (p_callback.compare(p, max) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        p_callback.swap(max, hi);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl swap (
    char *a,
    char *b,
    size_t width
    )
{
    char tmp;

    if ( a != b )
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}


PFC_DLL_EXPORT void sort_void(void * base,t_size num,t_size width,int (*comp)(const void *, const void *) )
{
	sort_void_ex(base,num,width,comp,swap_void);
}




sort_callback_stabilizer::sort_callback_stabilizer(sort_callback & p_chain,t_size p_count)
: m_chain(p_chain)
{
	m_order.set_size(p_count);
	t_size n;
	for(n=0;n<p_count;n++) m_order[n] = n;
}

int sort_callback_stabilizer::compare(t_size p_index1, t_size p_index2) const
{
	int ret = m_chain.compare(p_index1,p_index2);
	if (ret == 0) ret = pfc::sgn_t((t_ssize)m_order[p_index1] - (t_ssize)m_order[p_index2]);
	return ret;
}

void sort_callback_stabilizer::swap(t_size p_index1, t_size p_index2)
{
	m_chain.swap(p_index1,p_index2);
	pfc::swap_t(m_order[p_index1],m_order[p_index2]);
}


PFC_DLL_EXPORT void sort_stable(sort_callback & p_callback,t_size p_count)
{
	sort(sort_callback_stabilizer(p_callback,p_count),p_count);
}

}

