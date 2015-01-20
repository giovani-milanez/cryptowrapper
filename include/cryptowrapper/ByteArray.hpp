/*
 * ByteArray.hpp
 *
 *      Author: Giovani Milanez Espindola
 *	   Contact: giovani.milanez@gmail.com
 *	Created on: 03/09/2013
 */

#ifndef cryptowrapper_BYTEARRAY_H_
#define cryptowrapper_BYTEARRAY_H_

#include "cryptowrapper/Buffer.hpp"

#include <fstream>

namespace cryptowrapper {

class CRYPTOWRAPPER_API ByteArray : public Buffer<unsigned char>
{
public:
	using Buffer<unsigned char>::Buffer;

	static ByteArray createFromFile(const std::string& filename);
	std::string hex(const ByteArray& ba);
};

std::ostream& operator<<( std::ostream& os, const ByteArray& buffer );

}

#endif /* BYTEARRAY_H_ */
