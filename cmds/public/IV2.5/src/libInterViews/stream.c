/*
 * C++ stream I/O
 */

#include <ctype.h>
#include <stdlib.h>
#include <stream.h>
#include <string.h>

filebuf cin_filebuf(stdin);
istream cin(&cin_filebuf, 1, &cout);

filebuf cout_filebuf(stdout);
ostream cout(&cout_filebuf);

filebuf cerr_filebuf(stderr);
ostream cerr(&cerr_filebuf);

whitespace WS;

enum stream_status {
    stream_good = 0, stream_eof = 1, stream_fail = 2, stream_bad = 4
};

/*
 * Stream buffer implementation.
 */

streambuf::streambuf () {
    _base = nil;
    _free = nil;
    _used = nil;
    _end = nil;
    _allocated = false;
    _fp = nil;
}

streambuf::streambuf (char* base, int length) {
    setbuf(base, length);
    _allocated = false;
}

streambuf::~streambuf () {
    if (_base != nil && _allocated) {
	delete _base;
    }
}

int streambuf::fill () {
    return EOF;
}

int streambuf::flush (int c) {
    if (!allocate()) {
	return EOF;
    }
    if (c != EOF) {
	*_free++ = c;
    }
    return c&0xff;
}

int streambuf::sgetc () {
    return (_used >= _free) ? fill() : (*_used&0xff);
}

int streambuf::snextc () {
    return (_used >= _free-1) ? fill() : (*++_used&0xff);
}

void streambuf::stossc () {
    register char* p = _used;
    ++_used;
    if (p >= _free) {
	fill();
    }
}

void streambuf::sputbackc (char c) {
    if (_used > _base) {
	*--_used = c;
    }
}

int streambuf::sputc (int c) {
    register int i;
    if (_fp == nil) {
	i = c&0xff;
	if (_free >= _end) {
	    i = flush(i);
	} else {
	    *_free++ = i;
	}
    } else {
	i = putc(c, _fp);
    }
    return i;
}

streambuf* streambuf::setbuf (char* base, int length, int count) {
    _base = base;
    _used = base;
    _free = base + count;
    _end = base + length;
    return this;
}

boolean streambuf::allocate () {
    if (_base != nil) {
	return true;
    }
    _base = new char[BUFSIZ];
    if (_base == nil) {
	return false;
    }
    _free = _base;
    _used = _base;
    _end = _base + BUFSIZ;
    _allocated = true;
    return true;
}

/*
 * A file buffer is a stream buffer that is associated with an open file.
 */

filebuf::filebuf () {
    _isopen = false;
    _fp = nil;
}

filebuf::filebuf (FILE* f) {
    _isopen = true;
    _fp = f;
}

filebuf::filebuf (int fd) {
    _isopen = true;
    _fd = fd;
}

filebuf::filebuf (int fd, char* base, int length) : (base, length) {
    _isopen = true;
    _fd = fd;
}

filebuf::~filebuf() {
    close();
}

filebuf* filebuf::open (char* name, open_mode m) {
    switch (m) {
	case input:
	    _fd = ::open(name, 0);
	    break;
	case output:
	    _fd = ::creat(name, 0666);
	    break;
	case append:
	    _fd = ::open(name, 1);
	    if (_fd >= 0) {
		::lseek(_fd, 0, 2);
	    } else {
		_fd = creat(name, 0666);
	    }
	    break;
    }
    if (_fd < 0) {
	return nil;
    }
    _isopen = true;
    return this;
}

int filebuf::close () {
    if (_isopen) {
	_isopen = false;
	return ::close(_fd);
    }
    return 0;
}

int filebuf::fill () {
    if (!_isopen || !allocate()) {
	return EOF;
    }
    int nread;
    char* start = _base + 1;
    if (_fp != nil) {
	if (fgets(start, _end - start, _fp) == nil) {
	    return EOF;
	}
	nread = strlen(start);
    } else {
	nread = ::read(_fd, start, _end - start);
	if (nread <= 0) {
	    return EOF;
	}
    }
    _used = start;
    _free = start + nread;
    return *_used&0xff;
}

int filebuf::flush (int c) {
    if (!_isopen || !allocate()) {
	return EOF;
    }
    if (_fp != nil) {
	fflush(_fp);
	return 0;
    }
    if (_base == _end) {
	if (c != EOF) {
	    char cc = c;
	    ::write(_fd, &cc, 1);
	}
    } else {
	if (_free > _base) {
	    ::write(_fd, _base, _free - _base);
	    _free = _base;
	    _used = _base;
	}
	if (c != EOF) {
	    *_free++ = c;
	}
    }
    return c&0xff;
}

/*
 * A circular buffer is a simple buffer that wraps around.
 * It does no I/O.
 */

int circbuf::fill () {
    return EOF;
}

int circbuf::flush (int c) {
    if (!allocate()) {
	return EOF;
    }
    _free = _base;
    if (c != EOF) {
	*_free++ = c;
    }
    return c&0xff;
}

/*
 * iostream is the base for istream and ostream.
 * It is an abstract class (no instances) and just implements
 * some common operations relating to the status of the stream.
 */

iostream::iostream () {
    _buf = new streambuf;
    _status = stream_good;
}

iostream::iostream (int fd) {
    _buf = new filebuf(fd);
    _status = stream_good;
}

iostream::iostream (streambuf* b) {
    _buf = b;
    _status = stream_good;
}

boolean iostream::operator ! () {
    return fail();
}

boolean iostream::good () {
    return status() == stream_good;
}

boolean iostream::eof () {
    return (status() & stream_eof) != 0;
}

boolean iostream::fail () {
    return status() > stream_eof;
}

boolean iostream::bad () {
    return (status() & stream_bad) != 0;
}

ostream::ostream (int fd) : (fd) {}

ostream::ostream (int size, char* base) {
    _buf->setbuf(base == nil ? new char[size] : base, size);
}

ostream::ostream (streambuf* b) : (b) {}

ostream::~ostream () {
    flush();
}

ostream& ostream::operator << (const char* str) {
    if (good() && str != nil && *str != '\0') {
	register streambuf* p = _buf;
	for (register const char* s = str; *s != '\0'; s++) {
	    if (p->sputc(*s) == EOF) {
		status(stream_eof | stream_fail);
		break;
	    }
	}
    }
    return *this;
}

ostream& ostream::put(char c) {
    if (good() && _buf->sputc(c) == EOF) {
	status(stream_eof | stream_fail);
    }
    return *this;
}

ostream& ostream::operator << (long i) {
    if (good()) {
	char tmp[100];
	sprintf(tmp, "%ld", i);
	*this << tmp;
    }
    return *this;
}

ostream& ostream::operator << (unsigned long i) {
    if (good()) {
	char tmp[100];
	sprintf(tmp, "%lu", i);
	*this << tmp;
    }
    return *this;
}

ostream& ostream::operator << (double d) {
    if (good()) {
	char tmp[100];
	sprintf(tmp, "%g", d);
	*this << tmp;
    }
    return *this;
}

ostream& ostream::operator << (const streambuf& b) {
    if (good()) {
	register streambuf* p = _buf;
	for (register int c = b.sgetc(); c != EOF; c = b.snextc()) {
	    if (p->sputc(c) == EOF) {
		status(stream_eof | stream_fail);
		break;
	    }
	}
    }
    return *this;
}

ostream& ostream::put (const char* fmt, char c) {
    char tmp[10];
    sprintf(tmp, fmt, c);
    return *this << tmp;
}

ostream& ostream::put (const char* fmt, char* s) {
    char tmp[1024];
    sprintf(tmp, fmt, s);
    return *this << tmp;
}

ostream& ostream::put (const char* fmt, long i) {
    char tmp[100];
    sprintf(tmp, fmt, i);
    return *this << tmp;
}

ostream& ostream::put (const char* fmt, unsigned long i) {
    char tmp[100];
    sprintf(tmp, fmt, i);
    return *this << tmp;
}

ostream& ostream::put (const char* fmt, double d) {
    char tmp[100];
    sprintf(tmp, fmt, d);
    return *this << tmp;
}

ostream& ostream::flush () {
    _buf->flush();
    return *this;
}

istream::istream (int fd, boolean skip, ostream* tied) : (fd) {
    _skipws = skip;
    _tied = tied;
}

istream::istream (int size, char* base, boolean skip) {
    _buf->setbuf(base == nil ? new char[size] : base, size, size);
    _skipws = skip;
    _tied = nil;
}

istream::istream (streambuf* b, boolean skip, ostream* tied) : (b) {
    _skipws = skip;
    _tied = tied;
}

boolean istream::skip (boolean s) {
    boolean prev = _skipws;
    _skipws = s;
    return prev;
}

inline void istream::sync () {
    if (_tied != nil) {
	_tied->flush();
    }
    if (_skipws) {
	doskip();
    }
}

void istream::doskip () {
    register streambuf* p = _buf;
    for (register int c = p->sgetc(); isspace(c&0xff); c = p->snextc());
    if (c == EOF) {
	status(stream_eof);
    }
}

istream& istream::operator >> (char* str) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	register char* s = str;
	register int c = p->sgetc();
	if (c == EOF) {
	    status(stream_fail);
	} else {
	    for (; !isspace(c) && c != EOF; c = p->snextc()) {
		*s++ = c;
	    }
	}
	*s = '\0';
	if (c == EOF) {
	    status(stream_eof);
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::operator >> (char& s) {
    sync();
    if (good()) {
	register int c = _buf->sgetc();
	if (c == EOF) {
	    status(stream_eof | stream_fail);
	} else {
	    s = c;
	    _buf->stossc();
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::operator >> (short& s) {
    long l;
    if (*this >> l) {
	s = (short)l;
    }
    return *this;
}

istream& istream::operator >> (int& i) {
    long l;
    if (*this >> l) {
	i = (int)l;
    }
    return *this;
}

istream& istream::operator >> (long& n) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	char num[1024];
	register char* np = num;
	register int c = p->sgetc();
	if (c == '+' || c == '-') {
	    *np++ = c;
	    c = p->snextc();
	}
	if (isdigit(c)) {
	    do {
		*np++ = c;
		c = p->snextc();
	    } while (isdigit(c));
	    if ((c == 'x' || c == 'X') && np == num + 1 && num[0] == '0') {
		do {
		    *np++ =c;
		    c = p->snextc();
		} while (isdigit(c));
	    }
	    if (c == 'l' || c == 'L') {
		p->snextc();
	    }
	    *np = '\0';
	    n = atol(num);
	} else {
	    status(stream_fail);
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::operator >> (float& f) {
    double d;
    if (*this >> d) {
	f = (float)d;
    }
    return *this;
}

istream& istream::operator >> (double& d) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	char num[1024];
	register char* np = num;
	register int c = p->sgetc();
	if (c == '+' || c == '-') {
	    *np++ = c;
	    c = p->snextc();
	}
	for (; isdigit(c); c = p->snextc()) {
	    *np++ = c;
	}
	if (c == '.') {
	    do {
		*np++ = c;
		c = p->snextc();
	    } while (isdigit(c));
	}
	if (c == 'e' || c == 'E') {
	    *np++ = c;
	    c = p->snextc();
	    if (c == '+' || c == '-') {
		*np++ = c;
		c = p->snextc();
	    }
	    for (; isdigit(c); c = p->snextc()) {
		*np++ = c;
	    }
	}
	*np = '\0';
	d = atof(num);
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::operator >> (streambuf& b) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	register int c = p->sgetc();
	if (c == EOF) {
	    status(stream_eof | stream_fail);
	} else {
	    for (; c != EOF && b.sputc(c) != EOF; c = p->snextc());
	    if (c == EOF) {
		status(stream_eof);
	    }
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::operator >> (whitespace&) {
    if (good()) {
	register streambuf* p = _buf;
	if (_tied != nil) {
	    _tied->flush();
	}
	for (register int c = p->sgetc(); isspace(c); c = p->snextc());
	if (c == EOF) {
	    status(stream_eof);
	}
    }
    return *this;
}

istream& istream::get (char* str, int n, char e) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	register int c = p->sgetc();
	if (c == EOF) {
	    status(stream_fail | stream_eof);
	} else {
	    register char* s = str;
	    register int i;
	    for (i = 0; c != e && c != EOF && i < n; i++, c = p->snextc()) {
		*s++ = c;
	    }
	    *s = '\0';
	    if (c == EOF) {
		status(stream_eof);
	    }
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::get (streambuf& b, char e) {
    sync();
    if (good()) {
	register streambuf* p = _buf;
	register int c = p->sgetc();
	if (c == EOF) {
	    status(stream_fail | stream_eof);
	} else {
	    for (; c != e && c != EOF && b.sputc(c) != EOF; c = p->snextc());
	    if (c == EOF){
		status(stream_eof);
	    }
	}
    } else {
	status(stream_fail);
    }
    return *this;
}

istream& istream::get (char& c) {
    register boolean save = _skipws;
    _skipws = false;
    *this >> c;
    _skipws = save;
    return *this;
}

istream& istream::putback (char c) {
    _buf->sputbackc(c);
    return *this;
}

ostream* istream::tie (ostream* s) {
    ostream* old = _tied;
    _tied = s;
    return old;
}
