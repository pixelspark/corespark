#include "../include/tjshared.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
using namespace tj::shared;
using namespace std;

void encode(unsigned long *v, unsigned long *k) {              
	unsigned long y=v[0],z=v[1], sum=0, delta=0x9e3779b9, n=32;
	while(n-->0) {
		sum += delta;
		y += (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1];
		z += (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3];
	}

    k[2]^=y;	k[3]^=z;
	k[0]^=v[0]; k[1]^=v[1];
	v[0]=y;		v[1]=z;
}

void decode(unsigned long *v, unsigned long *k) {
	unsigned long n=32, y=v[0], z=v[1], delta=0x9e3779b9, sum=delta<<5;

	while(n-->0) {
		z-= (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3] ; 
		y-= (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1] ;
		sum-=delta;
	}

	k[0]^=y;	k[1]^=z;
	k[2]^=v[0];	k[3]^=v[1];
	v[0]=y;		v[1]=z;
}

template <typename InputIterator, typename OutputIterator> OutputIterator encrypt(InputIterator pos, const InputIterator &end, OutputIterator &out, unsigned long key[4]){
	unsigned long pad[2];
    
	while(pos != end) {
		pad[0] = pad[1] = 0;
		if(pos != end) pad[0] |= (*pos++)<<0;  else pad[0] |= ' '<<0;
		if(pos != end) pad[0] |= (*pos++)<<8;  else pad[0] |= ' '<<8;
		if(pos != end) pad[0] |= (*pos++)<<16; else pad[0] |= ' '<<16;
		if(pos != end) pad[0] |= (*pos++)<<24; else pad[0] |= ' '<<24;
		if(pos != end) pad[1] |= (*pos++)<<0;  else pad[1] |= ' '<<0;
		if(pos != end) pad[1] |= (*pos++)<<8;  else pad[1] |= ' '<<8;
		if(pos != end) pad[1] |= (*pos++)<<16; else pad[1] |= ' '<<16;
		if(pos != end) pad[1] |= (*pos++)<<24; else pad[1] |= ' '<<24;
		encode(pad, key);
		*out++ = (const unsigned char)((pad[0]>>0)&0xff);
		*out++ = (const unsigned char)((pad[0]>>8)&0xff);
		*out++ = (const unsigned char)((pad[0]>>16)&0xff);
		*out++ = (const unsigned char)((pad[0]>>24)&0xff);
		*out++ = (const unsigned char)((pad[1]>>0)&0xff);
		*out++ = (const unsigned char)((pad[1]>>8)&0xff);
		*out++ = (const unsigned char)((pad[1]>>16)&0xff);
		*out++ = (const unsigned char)((pad[1]>>24)&0xff);
	}
    return out;
}

template <typename InputIterator, typename OutputIterator> OutputIterator decrypt(InputIterator pos, const InputIterator &end, OutputIterator &out, unsigned long key[4]) {
	unsigned long pad[2];

	while(pos != end) {
		pad[0]=0; pad[1]=0;
		if(pos != end) pad[0] |= (*pos++)<<0;  else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[0] |= (*pos++)<<8;  else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[0] |= (*pos++)<<16; else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[0] |= (*pos++)<<24; else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[1] |= (*pos++)<<0;  else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[1] |= (*pos++)<<8;  else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[1] |= (*pos++)<<16; else throw std::length_error("Block is incomplete.");
		if(pos != end) pad[1] |= (*pos++)<<24; else throw std::length_error("Block is incomplete.");
		decode(pad, key);
		*out++ = (const unsigned char)((pad[0]>>0)&0xff);
		*out++ = (const unsigned char)((pad[0]>>8)&0xff);
		*out++ = (const unsigned char)((pad[0]>>16)&0xff);
		*out++ = (const unsigned char)((pad[0]>>24)&0xff);
		*out++ = (const unsigned char)((pad[1]>>0)&0xff);
		*out++ = (const unsigned char)((pad[1]>>8)&0xff);
		*out++ = (const unsigned char)((pad[1]>>16)&0xff);
		*out++ = (const unsigned char)((pad[1]>>24)&0xff);
	}

	return out;
}

static const char base64_start[64] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S',
'T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l',
'm','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4',
'5','6','7','8','9','+','/'}, *base64_end = base64_start + 64;

template <typename InputIterator, typename OutputIterator> OutputIterator toBase64(InputIterator pos, const InputIterator &end, OutputIterator &out) {
	int bits = 0;
	unsigned long value = 0;

	for(; pos != end; ++pos) {
		value = (value<<8)|*pos;
		bits += 8;
        do {
			*out++ = base64_start[(value>>(bits-6))&((1<<6)-1)];
			bits -= 6;
		} 
		while(bits >= 6);
	}

	if(bits) {
		*out++ = base64_start[(value<<(6-bits))&((1<<6)-1)];
	}

	return out;
}

template <typename T> unsigned char toValue(T value) {
	const char *r = std::find(base64_start, base64_end, value);
	if(r == base64_end) throw std::out_of_range("Not base64 character.");
	return (unsigned char)(r - base64_start);
}

template <typename InputIterator, typename OutputIterator> OutputIterator fromBase64(InputIterator pos, const InputIterator &end, OutputIterator &out) {
	int bits = 0;
	unsigned long value = 0;

	for(; pos != end; ++pos) {
		unsigned char code;
        try {
			code = toValue(*pos);
		}
		catch(const std::out_of_range &) {
			if(isspace(*pos)) {
				continue;
			}
			else throw;
		}

		value = (value<<6) | code;
		bits+=6;

		if(bits >= 8) {
			*out++ = (const unsigned char)((value>>(bits-8))&((1<<8)-1));
			bits -= 8;
		}
	}
	
	return out;
}

void stringToKey(const std::string &key, unsigned long output_key[4]) { 
	unsigned long fake_key[4] = {0x01234567, 0x89abcdef};

	std::vector<unsigned char> output;
	std::insert_iterator<std::vector<unsigned char> > out(output, output.end());

	encrypt(key.begin(), key.end(), out, fake_key);

	output_key[0] = output_key[1] = output_key[2] = output_key[3] = 0;

	int i = 0;
	for(std::vector<unsigned char>::const_iterator c = output.begin(); c != output.end(); ++c, ++i) {
		output_key[(i%16)/4] = *c << (8*(i%4));
	}
}

void decryptMessage(const std::string &intstr, std::string &outstr, const std::string &key) {
	std::vector<unsigned char> output;
	std::insert_iterator<std::vector<unsigned char> > out(output, output.end());
	fromBase64(intstr.begin(), intstr.end(), out);

	unsigned long keybuf[4];
	stringToKey(key, keybuf);

	std::string doutput;
	std::insert_iterator<std::string> dout(doutput, doutput.end());
	decrypt(output.begin(), output.end(), dout, keybuf);

	outstr = doutput;
}

void encryptMessage(const std::string &intstr, std::string &outstr, const std::string &key) {
	unsigned long keybuf[4];
	stringToKey(key, keybuf);

	std::vector<unsigned char> output;
	std::insert_iterator<std::vector<unsigned char> > out(output, output.end());
	encrypt(intstr.begin(), intstr.end(), out, keybuf);

	std::string coutput;
	std::insert_iterator<std::string> cout(coutput, coutput.end());
	toBase64(output.begin(), output.end(), cout);

	outstr = coutput;
}

/* TEA Class */
TEAEncrypter::TEAEncrypter() {
}

TEAEncrypter::~TEAEncrypter() {
}

std::string TEAEncrypter::Encrypt(std::string src, std::string key) {
	std::string out;
	encryptMessage(src, out, key);
	return out;
}

std::string TEAEncrypter::Decrypt(std::string src, std::string key) {
	std::string out;
	decryptMessage(src,out,key);
	return out;
}
