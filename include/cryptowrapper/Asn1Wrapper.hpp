/*
 * Asn1Wrapper.hpp
 *
 *  Criado em: 28/07/2014
 *      Autor: Giovani Milanez Espindola
 *  Contact: giovani.milanez@gmail.com
 */

#ifndef ASN1WRAPPER_HPP_
#define ASN1WRAPPER_HPP_

#include "cryptowrapper/ByteArray.hpp"
#include "cryptowrapper/Exception.hpp"

#include <openssl/bio.h>
#include <openssl/pem.h>

#include <memory>

namespace cryptowrapper {

template
	<
		typename T, // the openssl struct to wrap, e.g. X509
		T* (*D2I)(T**, const unsigned char **, long), //der to internal function, e.g. d2i_X509
		int (*I2D)(T*, unsigned char **), // internal do der function, e.g. i2d_X509
		void (*FreeFunc)(T*),  // free function, e.g. X509_free
		T* (*DupFunc)(T*), // dup funcion, e.g. X509_dup
		int (*PemWrite)(BIO *, T *) = nullptr, // optional pem write funcion, e.g. PEM_write_bio_X509. If not specified getPemEncoded() function will throw.
		T* (*PemRead)(BIO *, T **, pem_password_cb *, void *) = nullptr, // optional pem read function, e.g. PEM_read_bio_X509. If not specified will always try to decode DER. If specified try both.
		int	(*CMP)(const T *a, const T *b) = nullptr // optional compare function, e.g. X509_cmp. If not specified operator== will compare the structs DER encoded
	>
class Asn1Object
{
public:

	static T* fromFile(const std::string& filePath)
	{
		return fromFile(filePath, nullptr, nullptr);
	}

	static T* fromFile(const std::string& filePath, pem_password_cb *cb)
	{
		return fromFile(filePath, cb, nullptr);
	}

	static T* fromFile(const std::string& filePath, pem_password_cb *cb, void *userData)
	{
		return fromDerOrPem(ByteArray::createFromFile(filePath), cb, userData);
	}

	static T* fromDerOrPem(const ByteArray& derOrPemEncoded)
	{
		return fromDerOrPem(derOrPemEncoded, nullptr, nullptr);
	}

	static T* fromDerOrPem(const ByteArray& derOrPemEncoded, pem_password_cb *cb)
	{
		return fromDerOrPem(derOrPemEncoded, cb, nullptr);
	}

	static T* fromDerOrPem(const ByteArray& derOrPemEncoded, pem_password_cb *cb, void *userData)
	{
		const unsigned char *tmp = derOrPemEncoded.begin();
		T* internal_ = D2I(nullptr, &tmp, derOrPemEncoded.size());
		if (internal_ == nullptr) // der decode failed.
		{
			if(PemRead == nullptr) // we dont have PEM_read function, lets end here.
				throw cryptowrapper::DerDecodeException("");

			// der failed, try pem
			std::unique_ptr<BIO, std::function<void(BIO*)>> buffer(BIO_new(BIO_s_mem()), [](BIO *ptr){BIO_free(ptr);});
			if ((unsigned int)(BIO_write(buffer.get(), derOrPemEncoded.begin(), derOrPemEncoded.size())) != derOrPemEncoded.size())
			{
				throw cryptowrapper::BufferWriteException("Could not write PEM to buffer");
			}
			internal_ = PemRead(buffer.get(), nullptr, cb, userData);
			if (internal_ == nullptr)
			{
				throw cryptowrapper::DecodeException("DER and PEM decode failed"); // we tried DER and PEM decoding, both failed.
			}
		}
		return internal_;
	}

	Asn1Object(T *p) :
        internal_(p)
	{
		if(p == nullptr)
			throw cryptowrapper::NullPointerException("Trying to construct Asn1Object with null pointer");
	}
    virtual ~Asn1Object()
    {
		FreeFunc(internal_);
    }

    Asn1Object(const Asn1Object& src) :
        internal_(DupFunc(src.internal_))
    {
    }
    Asn1Object& operator=(const Asn1Object& rhs)
    {
        if (this == &rhs)
            return *this;

        T *tmp = DupFunc(rhs.internal_);
        FreeFunc(internal_);
        internal_ = tmp;
        return *this;
    }
    Asn1Object(Asn1Object&& src) :
        internal_(src.internal_)
    {
        src.internal_ = nullptr;
    }
    Asn1Object& operator=(Asn1Object&& rhs)
    {
        if (this == &rhs)
            return *this;

        FreeFunc(internal_);
        internal_ = rhs.internal_;
        rhs.internal_ = nullptr;
        return *this;
    }

    Asn1Object clone() const
    {
    	return Asn1Object(DupFunc(internal_));
    }

    T* cloneInternal() const
    {
    	return DupFunc(internal_);
    }

    T* internal() const
    {
    	return internal_;
    }

	T* steal()
	{
		T* tmp = internal_;
		internal_ = nullptr;
		return tmp;
	}

	operator const T *() const
	{
		return internal_;
	}

	T* operator->() const
	{
		return internal_;
	}

    ByteArray getDerEncoded() const
	{
        std::size_t size = I2D(internal_, nullptr);
        cryptowrapper::ByteArray result(size);
        unsigned char *derPtr = result.begin();
        unsigned char *tmp = derPtr;
        I2D(internal_, &tmp);
        return result;
	}

	std::string getPemEncoded() const
	{
		if(PemWrite == nullptr)
			throw cryptowrapper::RuntimeException("No PemWrite function provided for this wrapper");

		const char *data;
		BIO *buffer = BIO_new(BIO_s_mem());
		PemWrite(buffer, internal_);
		std::size_t ndata = BIO_get_mem_data(buffer, &data);
		std::string ret(data, ndata);
		BIO_free(buffer);
		return ret;
	}

	static bool hasPemDecode()
	{
		return PemRead != nullptr;
	}

	static bool hasPemEncode()
	{
		return PemWrite != nullptr;
	}

	bool operator ==(const Asn1Object& value) const
	{
		if(CMP == nullptr)
			return getDerEncoded() == value.getDerEncoded();

		return CMP(*this, value) == 0;
	}

	bool operator !=(const Asn1Object& value) const
	{
		return !this->operator==(value);
	}

protected:
	T *internal_;
};

}

#define EXPAND_ASN1(struct_name) cryptowrapper::Asn1Object<struct_name, d2i_##struct_name, i2d_##struct_name, struct_name##_free, struct_name##_dup>
#define EXPAND_ASN1_CMP(struct_name) cryptowrapper::Asn1Object<struct_name, d2i_##struct_name, i2d_##struct_name, struct_name##_free, struct_name##_dup, struct_name##_cmp>
#define EXPAND_ASN1_PEM(struct_name) cryptowrapper::Asn1Object<struct_name, d2i_##struct_name, i2d_##struct_name, struct_name##_free, struct_name##_dup, PEM_write_bio_##struct_name, PEM_read_bio_##struct_name>
#define EXPAND_ASN1_PEM_CMP(struct_name) cryptowrapper::Asn1Object<struct_name, d2i_##struct_name, i2d_##struct_name, struct_name##_free, struct_name##_dup, PEM_write_bio_##struct_name, PEM_read_bio_##struct_name, struct_name##_cmp>


#endif /* ASN1WRAPPER_HPP_ */
