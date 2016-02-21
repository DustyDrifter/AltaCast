#include <math.h>

#ifdef _M_X64
#include <xmmintrin.h>
#include <emmintrin.h>
#endif


#define audio_sample_size 32

#if audio_sample_size == 32
typedef float audio_sample;
#define audio_sample_asm dword
#elif audio_sample_size == 64
typedef double audio_sample;
#define audio_sample_asm qword
#else
#error wrong audio_sample_size
#endif

#define audio_sample_bytes (audio_sample_size/8)

namespace audio_math
{
	//! p_source/p_output can point to same buffer
	void SHARED_EXPORT scale(const audio_sample * p_source,t_size p_count,audio_sample * p_output,audio_sample p_scale);
	void SHARED_EXPORT convert_to_int16(const audio_sample * p_source,t_size p_count,t_int16 * p_output,audio_sample p_scale);
	void SHARED_EXPORT convert_to_int32(const audio_sample * p_source,t_size p_count,t_int32 * p_output,audio_sample p_scale);
	audio_sample SHARED_EXPORT convert_to_int16_calculate_peak(const audio_sample * p_source,t_size p_count,t_int16 * p_output,audio_sample p_scale);
	void SHARED_EXPORT convert_from_int16(const t_int16 * p_source,t_size p_count,audio_sample * p_output,audio_sample p_scale);
	void SHARED_EXPORT convert_from_int32(const t_int32 * p_source,t_size p_count,audio_sample * p_output,audio_sample p_scale);
	audio_sample SHARED_EXPORT convert_to_int32_calculate_peak(const audio_sample * p_source,t_size p_count,t_int32 * p_output,audio_sample p_scale);
	audio_sample SHARED_EXPORT calculate_peak(const audio_sample * p_source,t_size p_count);
	void SHARED_EXPORT remove_denormals(audio_sample * p_buffer,t_size p_count);
	void SHARED_EXPORT add_offset(audio_sample * p_buffer,audio_sample p_delta,t_size p_count);

	
	inline t_uint64 time_to_samples(double p_time,t_uint32 p_sample_rate) {
		return (t_uint64)floor((double)p_sample_rate * p_time + 0.5);
	}

	inline double samples_to_time(t_uint64 p_samples,t_uint32 p_sample_rate) {
		PFC_ASSERT(p_sample_rate > 0);
		return (double) p_samples / (double) p_sample_rate;
	}


#ifdef _M_IX86
	inline static t_int64 rint64(audio_sample val) {
		t_int64 rv;
		_asm {
			fld val;
			fistp rv;
		}
		return rv;
	}
	inline static t_int32 rint32(audio_sample val) {
		t_int32 rv; 
		_asm {
			fld val;
			fistp rv;
		}
		return rv;
	}
#elif defined(_M_X64)
	inline static t_int64 rint64(audio_sample val) {return (t_int64)floor(val+0.5);}
	static inline t_int32 rint32(float p_val) {
		return (t_int32)_mm_cvtss_si32(_mm_load_ss(&p_val));
	}
#else
	inline static t_int64 rint64(audio_sample val) {return (t_int64)floor(val+0.5);}
	inline static t_int32 rint32(audio_sample val) {return (t_int32)floor(val+0.5);}
#endif


	inline audio_sample gain_to_scale(double p_gain) {return (audio_sample) pow(10.0,p_gain / 20.0);}
	inline double scale_to_gain(double scale) {return 20.0*log10(scale);}


	static const audio_sample float16scale = 65536.f;

	inline audio_sample decodeFloat24ptr(const void * sourcePtr) {
		PFC_STATIC_ASSERT(pfc::byte_order_is_little_endian);
		union {
			uint8_t bytes[4];
			float v;
		} u;
		const uint8_t * s = reinterpret_cast<const uint8_t*>(sourcePtr);
		u.bytes[0] = 0;
		u.bytes[1] = s[0];
		u.bytes[2] = s[1];
		u.bytes[3] = s[2];
		return u.v;
	}
	inline audio_sample decodeFloat24ptrbs(const void * sourcePtr) {
		PFC_STATIC_ASSERT(pfc::byte_order_is_little_endian);
		union {
			uint8_t bytes[4];
			float v;
		} u;
		const uint8_t * s = reinterpret_cast<const uint8_t*>(sourcePtr);
		u.bytes[0] = 0;
		u.bytes[1] = s[2];
		u.bytes[2] = s[1];
		u.bytes[3] = s[0];
		return u.v;
	}

	inline audio_sample decodeFloat16(uint16_t source) {
		const unsigned fractionBits = 10;
		const unsigned widthBits = 16;
		typedef uint16_t source_t;

	/*	typedef uint64_t out_t; typedef double retval_t;
		enum { 
			outExponent = 11, 
			outFraction = 52, 
			outExponentShift = (1 << (outExponent-1))-1 
		};*/

		typedef uint32_t out_t; typedef float retval_t;
		enum { 
			outExponent = 8, 
			outFraction = 23, 
			outExponentShift = (1 << (outExponent-1))-1 
		};

		const unsigned exponentBits = widthBits - fractionBits - 1;
		// 1 bit sign | exponent | fraction
		source_t fraction = source & (((source_t)1 << fractionBits)-1);
		source >>= fractionBits;
		int exponent = (int)( source & (((source_t)1 << exponentBits)-1) ) - (int)((1 << (exponentBits-1))-1);
		source >>= exponentBits;

		if (outExponent + outExponentShift <= 0) return 0;

		out_t output = (out_t)( source&1 );
		output <<= outExponent;
		output |= (unsigned) (exponent + outExponentShift) & ( (1<<outExponent) - 1 );
		output <<= outFraction;
		int shift = (int) outFraction - (int) fractionBits;
		if (shift < 0) output |= (out_t) (fraction >> -shift);
		else output |= (out_t) (fraction << shift);
		return *(retval_t*)&output / float16scale;
	}


}