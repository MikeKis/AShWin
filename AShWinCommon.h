#pragma once

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <list>
#include <deque>

#ifdef FOR_LINUX
#include <inttypes.h>
typedef __uint64_t UNS64;
#define sscanf_s sscanf
#define DYNAMIC_LIBRARY_ENTRY_POINT extern "C"
#else 
typedef unsigned __int64 UNS64;
#define DYNAMIC_LIBRARY_ENTRY_POINT __declspec(dllexport)
#endif

#include "pugixml-1.10/src/pugixml.hpp"

#define FOR_(i,a) for(i=0;i<(a);i++)
#define FORI(N) for(UNS64 _i = 0; _i < (UNS64)(N); _i++)

#ifndef FUNS64

inline int atoi_s(const char *pch)
{
    if (!*pch)
        return 0;
    int ret;
    if (sscanf_s(pch, "%d", &ret) != 1)
        throw std::runtime_error("int format conversion error");
    return ret;
}

inline std::string str(int i) {
	char buf[30];
#ifndef FOR_LINUX
	sprintf_s(buf, sizeof(buf), "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	return(std::string(buf));
}

#define STRING std::string
#define VECTOR std::vector
#define MAP std::map
#define PAIR std::pair
#define SET std::set
#define LIST std::list
#define UNARY_FUNCTION std::unary_function
#define BINARY_FUNCTION std::binary_function
#define MULTIMAP std::multimap
#define DEQUE std::deque

class Serializer
{
    void *pContext;
    int   Version;
public:
    Serializer(): pContext(nullptr), Version(DefaultCurrentVersion) {};
    virtual void Write(void *p, size_t n) = 0;
    virtual void Read(void *p, size_t n) = 0;
    void SetContext(void *p) {pContext = p;}
    void *pGetContext(void) const {return(pContext);}
    void SetVersion(int StoredVersion) {Version = StoredVersion;}
    int GetVersion(void) const {return(Version);}
    static int DefaultCurrentVersion;
};

class FileSerializer: public Serializer
{
    FILE  *fil;
    size_t curoffset;   // for debug purposes only
public:
    FileSerializer(FILE *fil_)
    {
        fil = fil_;
        curoffset = 0;
    };
    virtual void Write(void *p, size_t n) override
    {
        if (curoffset <= 0x1267 && curoffset + n > 0x1267)
            printf("break\n");
        auto i = fwrite(p, 1, n, fil);
        if ((int)i != n)
            throw std::runtime_error("write error");
        curoffset += n;
    };
    virtual void Read(void *p, size_t n) override
    {
        auto i = fread(p, 1, n, fil);
        if ((int)i != n)
            throw std::runtime_error("read error");
        curoffset += n;
    };
    virtual ~FileSerializer() {};
};

class InStreamSerializer: public Serializer
{
    std::istream &is;
public:
    InStreamSerializer(std::istream &is_) : is(is_) {}
    virtual void Write(void *p, size_t n) override {throw std::logic_error("attempt to write to input stream serializer");};
    virtual void Read(void *p, size_t n) override
    {
        is.read((char *)p, n);
        if (!is.good())
            throw std::runtime_error("stream read error");
    };
    virtual ~InStreamSerializer() {};
};

class OutStreamSerializer : public Serializer
{
    std::ostream &os;
public:
    OutStreamSerializer(std::ostream &os_) : os(os_) {}
    virtual void Write(void *p, size_t n) override
    {
        os.write((const char *)p, n);
        if (!os.good())
            throw std::runtime_error("stream write error");
    };
    virtual void Read(void *p, size_t n) override {throw std::logic_error("attempt to read from output stream serializer");};
    virtual ~OutStreamSerializer() {};
};

template<class Scalar> Serializer &operator<<(Serializer &ser, const Scalar &a)
{
    ser.Write((void *)&a, sizeof(a));
    return(ser);
}

template<class Scalar> Serializer &operator>>(Serializer &ser, Scalar &a)
{
    ser.Read((void *)&a, sizeof(a));
    return(ser);
}

inline Serializer &operator<<(Serializer &ser, const STRING &str)
{
    int n = (int)str.length();
    ser.Write(&n, sizeof(n));
    ser.Write((void *)str.c_str(), n);
    return(ser);
}

inline Serializer &operator>>(Serializer &ser, STRING &str)
{
    int n;
    char *pch;
    ser.Read(&n, sizeof(n));
    pch = (char *)malloc(n + 1);
    if (!pch)
        throw std::runtime_error("out of memory");
    ser.Read(pch, n);
    pch[n] = '\0';
    str = pch;
    free(pch);
    return(ser);
}

inline int SerializationSize(STRING &str) {return((int)(sizeof(int) + str.length()));}

template<class Serializable> Serializer &operator<<(Serializer &ser, const VECTOR<Serializable> &v_)
{
    size_t n = v_.size();
    size_t i;
    ser.Write(&n, sizeof(n));
    for (i = 0; i < n; i++)
        ser << v_[i];
    return(ser);
}

template<class Serializable> Serializer &operator>>(Serializer &ser, VECTOR<Serializable> &v_)
{
    size_t n, i;
    ser.Read(&n, sizeof(n));
    v_.resize(n);
    for (i = 0; i < n; i++)
        ser >> v_[i];
    return(ser);
}

inline Serializer &operator<<(Serializer &ser, const VECTOR<bool> &vb_)
{
    size_t n = vb_.size();
    size_t i;
    ser.Write(&n, sizeof(n));
    for (i = 0; i < n; i++) {
        bool b = vb_[i];
        ser << b;
    }
    return(ser);
}

inline Serializer &operator>>(Serializer &ser, VECTOR<bool> &vb_)
{
    size_t n, i;
    ser.Read(&n, sizeof(n));
    vb_.resize(n);
    for (i = 0; i < n; i++) {
        bool b;
        ser >> b;
        vb_[i] = b;
    }
    return(ser);
}

template<class Serializable> Serializer &operator<<(Serializer &ser, const DEQUE<Serializable> &dq_)
{
    size_t n = dq_.size();
    size_t i;
    ser.Write(&n, sizeof(n));
    for (i = 0; i < n; i++)
        ser << dq_[i];
    return(ser);
}

template<class Serializable> Serializer &operator>>(Serializer &ser, DEQUE<Serializable> &dq_)
{
    size_t n, i;
    Serializable a;
    ser.Read(&n, sizeof(n));
    dq_.resize(n);
    for (i = 0; i < n; i++) {
        ser >> a;
        dq_[i] = a;
    }
    return(ser);
}

template<class Serializable> Serializer &operator<<(Serializer &ser, const std::unique_ptr<Serializable> &ptr)
{
    ser << *ptr;
    return(ser);
}

template<class Serializable> Serializer &operator>>(Serializer &ser, std::unique_ptr<Serializable> &ptr)
{
    ptr.reset(new Serializable);
    ser >> *ptr;
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator<<(Serializer &ser, const PAIR<Serializable1, Serializable2> &p_)
{
    ser << p_.first << p_.second;
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator>>(Serializer &ser, PAIR<Serializable1, Serializable2> &p_)
{
    ser >> p_.first >> p_.second;
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator<<(Serializer &ser, const MAP<Serializable1, Serializable2> &m_)
{
    int i;
    i = (int)m_.size();
    ser.Write(&i, sizeof(i));
    typename MAP<Serializable1, Serializable2>::const_iterator j;
    for (j = m_.begin(); j != m_.end(); j++)
        ser << *j;
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator>>(Serializer &ser, MAP<Serializable1, Serializable2> &m_)
{
    int n, i;
    ser.Read(&n, sizeof(n));
    m_.clear();
    FOR_(i, n) {
        Serializable1 a;
        ser >> a;
        ser >> m_[a];
    }
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator<<(Serializer &ser, const MULTIMAP<Serializable1, Serializable2> &mm_)
{
    int i;
    i = (int)mm_.size();
    ser.Write(&i, sizeof(i));
    typename MULTIMAP<Serializable1, Serializable2>::const_iterator j;
    for (j = mm_.begin(); j != mm_.end(); j++)
        ser << *j;
    return(ser);
}

template<class Serializable1, class Serializable2> Serializer &operator>>(Serializer &ser, MULTIMAP<Serializable1, Serializable2> &mm_)
{
    int n, i;
    PAIR<Serializable1, Serializable2> *pp_;
    ser.Read(&n, sizeof(n));
    pp_ = new PAIR<Serializable1, Serializable2>[n];
    FOR_(i, n)
        ser >> pp_[i];
    mm_.clear();
    copy(pp_, pp_ + i, inserter(mm_, mm_.end()));
    delete[] pp_;
    return(ser);
}

template<class T> void avgdis(const T *p, size_t n, double &davg, double &ddis)
{
	if (!n)
		throw std::runtime_error("avgdis");
	if (n == 1) {
		davg = *p;
		ddis = 0.;
		return;
	}
	size_t l;
	davg = 0;
	for (l = 0; l < n; l++)
		davg += p[l];
	davg /= n;
	ddis = 0;
	for (l = 0; l < n; l++) {
		double d = p[l] - davg;
		ddis += d * d;
	}
	ddis /= n - 1;
}

#endif

