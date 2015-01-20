# cryptowrapper
A OpenSSL crypto API wrapper for creating high level classes in C++


Whenever you use OpenSSL you have to deal with memory management, complex structs initilization and manipulation, 
resulting in ugly code. It is also hard when you want to use STL over that structs.

This project aim to easy those problems by providing a wrapper mechanism to generate high level classes.

Say you want to encapsulate the X509 structure, that represents a digital certificate.
You could write, using the convenient macro:
```c++
typedef EXPAND_ASN1_PEM_CMP(X509) Certificate;
```
the macro is expanded as follow
```c++
typedef Asn1Object<X509, d2i_X509, i2d_X509, X509_free, X509_dup, PEM_write_bio_X509, PEM_read_bio_X509, X509_cmp> Certificate;
```

If the struct you are trying to encapsulate does not support comparison function you should use
```
EXPAND_ASN1_PEM
```
If it does not support PEM write/read functions you should use
```
EXPAND_ASN1
or
EXPAND_ASN1_CMP (support comparison function)
```

What you get is a class containing:
 - Static methods for creating a X509* from a file path
 - Static methods for creating a X509* from a DER or PEM encoded certificate
 - A constructor receiving X509*
 - Automatic X509* cleanup on destructor (X509_free)
 - Copy constructor and assignment operator
 - Move constructor and move assignment
 - Clone method for X509* and for the class
 - operator ==
 - Class member access operator (->) overload, returning the X509*
 - Converstion operator to const X509*
 - getDerEncoded method (encode X509* to DER)
 - getPemEncoded method (encode X509* to PEM)

## Examples
Basic usage
```c++
#include "cryptowrapper/Asn1Wrapper.hpp"
#include <openssl/x509.h>
#include <iostream>

typedef EXPAND_ASN1_PEM_CMP(X509) Certificate;

int main()
{
	try
	{
		Certificate cert = Certificate::fromFile("/home/giovani/certificado/cert_giovani_2.cer");
		std::cout << cert.getPemEncoded() << std::endl;
		// direct access to X509*
		std::cout << cert->name << std::endl;		
	}
	catch(const cryptowrapper::Exception& ex)
	{
		std::cout << ex.displayText() << std::endl;
	}
}
```

Inheriting 
```c++
#include "cryptowrapper/Asn1Wrapper.hpp"
#include <openssl/x509.h>
#include <iostream>

class Certificate : public EXPAND_ASN1_PEM_CMP(X509)
{
public:
	// inheriting constructors
	using EXPAND_ASN1_PEM_CMP(X509)::Asn1Object;

	std::string getSubject() const
	{
		return internal_->name;
	}
};

int main()
{
	try
	{
		Certificate cert = Certificate::fromFile("/home/giovani/certificado/cert_giovani_2.cer");
		std::cout << cert.getSubject() << std::endl;
	}
	catch(const cryptowrapper::Exception& ex)
	{
		std::cout << ex.displayText() << std::endl;
	}
}
```
STL
```C++
#include "cryptowrapper/Asn1Wrapper.hpp"
#include <openssl/x509.h>
#include <iostream>
#include <vector>
#include <algorithm>

typedef EXPAND_ASN1_PEM_CMP(X509) Certificate;

int main()
{
	try
	{
		cryptowrapper::ByteArray memoryBa = /* load you cert */;
		Certificate certificateInMemory = Certificate::fromDerOrPem(memoryBa);

		std::vector<Certificate> certificates
		{
			Certificate::fromFile("..."),
			Certificate::fromFile("..."),
			certificateInMemory
		};
		for(auto& cert : certificates)
			std::cout << cert->name << std::endl;

		auto it = std::find(certificates.begin(), certificates.end(), certificateInMemory);
		if(it != certificates.end())
			std::cout << "Found!" << std::endl << it->getPemEncoded() << std::endl;
	}
	catch(const cryptowrapper::Exception& ex)
	{
		std::cout << ex.displayText() << std::endl;
	}
}
```
