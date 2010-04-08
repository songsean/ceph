// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */

#ifndef __OBJECT_H
#define __OBJECT_H

#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <iomanip>
using namespace std;

#include <ext/hash_map>
using namespace __gnu_cxx;

#include "hash.h"
#include "nstring.h"
#include "encoding.h"

struct object_t {
  nstring name;

  object_t(const char *s = 0) : name(s) {}
  object_t(const string& s) : name(s) {}
  void swap(object_t& o) {
    name.swap(o.name);
  }
  
  void encode(bufferlist &bl) const {
    ::encode(name, bl);
  }
  void decode(bufferlist::iterator &bl) {
    ::decode(name, bl);
  }
};
WRITE_CLASS_ENCODER(object_t)

inline bool operator==(const object_t& l, const object_t& r) {
  return l.name == r.name;
}
inline bool operator!=(const object_t& l, const object_t& r) {
  return l.name != r.name;
}
inline bool operator>(const object_t& l, const object_t& r) {
  return l.name > r.name;
}
inline bool operator<(const object_t& l, const object_t& r) {
  return l.name < r.name;
}
inline bool operator>=(const object_t& l, const object_t& r) { 
  return l.name >= r.name;
}
inline bool operator<=(const object_t& l, const object_t& r) {
  return l.name <= r.name;
}
inline ostream& operator<<(ostream& out, const object_t& o) {
  return out << o.name;
}

namespace __gnu_cxx {
  template<> struct hash<object_t> {
    size_t operator()(const object_t& r) const { 
      //static hash<nstring> H;
      //return H(r.name);
      return ceph_str_hash_linux(r.name.c_str(), r.name.length());
    }
  };
}


struct file_object_t {
  __u64 ino, bno;
  mutable char buf[33];

  file_object_t(__u64 i=0, __u64 b=0) : ino(i), bno(b) {
    buf[0] = 0;
  }
  
  const char *c_str() const {
    if (!buf[0])
      sprintf(buf, "%llx.%08llx", (long long unsigned)ino, (long long unsigned)bno);
    return buf;
  }

  operator object_t() {
    return object_t(c_str());
  }
};



// ---------------------------
// snaps

struct snapid_t {
  __u64 val;
  snapid_t(__u64 v=0) : val(v) {}
  snapid_t operator+=(snapid_t o) { val += o.val; return *this; }
  snapid_t operator++() { ++val; return *this; }
  operator __u64() const { return val; }  
};

inline void encode(snapid_t i, bufferlist &bl) { encode(i.val, bl); }
inline void decode(snapid_t &i, bufferlist::iterator &p) { decode(i.val, p); }

inline ostream& operator<<(ostream& out, snapid_t s) {
  if (s == CEPH_NOSNAP)
    return out << "head";
  else if (s == CEPH_SNAPDIR)
    return out << "snapdir";
  else
    return out << hex << s.val << dec;
}


struct sobject_t {
  object_t oid;
  snapid_t snap;

  sobject_t() : snap(0) {}
  sobject_t(object_t o, snapid_t s) : oid(o), snap(s) {}

  void swap(sobject_t& o) {
    oid.swap(o.oid);
    snapid_t t = snap;
    snap = o.snap;
    o.snap = t;
  }

  void encode(bufferlist& bl) const {
    ::encode(oid, bl);
    ::encode(snap, bl);
  }
  void decode(bufferlist::iterator& bl) {
    ::decode(oid, bl);
    ::decode(snap, bl);
  }
};
WRITE_CLASS_ENCODER(sobject_t)

inline bool operator==(const sobject_t l, const sobject_t r) {
  return l.oid == r.oid && l.snap == r.snap;
}
inline bool operator!=(const sobject_t l, const sobject_t r) {
  return l.oid != r.oid || l.snap != r.snap;
}
inline bool operator>(const sobject_t l, const sobject_t r) {
  return l.oid > r.oid || (l.oid == r.oid && l.snap > r.snap);
}
inline bool operator<(const sobject_t l, const sobject_t r) {
  return l.oid < r.oid || (l.oid == r.oid && l.snap < r.snap);
}
inline bool operator>=(const sobject_t l, const sobject_t r) { 
  return l.oid > r.oid || (l.oid == r.oid && l.snap >= r.snap);
}
inline bool operator<=(const sobject_t l, const sobject_t r) {
  return l.oid < r.oid || (l.oid == r.oid && l.snap <= r.snap);
}
inline ostream& operator<<(ostream& out, const sobject_t o) {
  return out << o.oid << "/" << o.snap;
}

namespace __gnu_cxx {
  template<> struct hash<sobject_t> {
    size_t operator()(const sobject_t &r) const { 
      static hash<object_t> H;
      static rjhash<uint64_t> I;
      return H(r.oid) ^ I(r.snap);
    }
  };
}

// ---------------------------

#endif
