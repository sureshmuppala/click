/*
 * fromdump.{cc,hh} -- element reads packets from tcpdump file
 * John Jannotti
 *
 * Copyright (c) 1999 Massachusetts Institute of Technology.
 *
 * This software is being provided by the copyright holders under the GNU
 * General Public License, either version 2 or, at your discretion, any later
 * version. For more information, see the `COPYRIGHT' file in the source
 * distribution.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "fromdump.hh"
#include "etheraddress.hh"
#include "confparse.hh"
#include "error.hh"
#include "glue.hh"

FromDump::FromDump()
  : _pcap(0), _pending_packet(0)
{
  add_output();
}

FromDump::FromDump(String filename)
  : _pcap(0), _pending_packet(0), _filename(filename)
{
  add_output();
}

FromDump::~FromDump() {
  if (_pcap) pcap_close(_pcap);
}

FromDump*
FromDump::clone() const
{
  return new FromDump(_filename);
}

int
FromDump::configure(const String &conf, ErrorHandler *errh)
{
  if (_pcap) pcap_close(_pcap);
  _pcap = 0;
  return cp_va_parse(conf, this, errh,
		     cpString, "dump file name", &_filename,
		     0);
}

int
FromDump::initialize(ErrorHandler *errh)
{
  if (_pcap)
    return 0;
  if (!_filename)
    return errh->error("filename not set");

#ifdef HAVE_PCAP
  char ebuf[PCAP_ERRBUF_SIZE];
  _pcap = pcap_open_offline((char*)(const char*)_filename, ebuf);
  if (!_pcap)
    return errh->error("pcap says: %s\n", ebuf);
#else
  errh->warning("can't read packets: not compiled with pcap support");
#endif
  timerclear(&_offset);
  return 0;
}

bool
FromDump::ready() {
#ifdef HAVE_PCAP
  if (!_pending_packet) {
    if (!pcap_dispatch(_pcap, 1, &FromDump::get_packet, (u_char*)this)) {
      return 0;
    }
  }
  // We now certainly have a pending packet.  Is it time to return it?
  timeval now;
  click_gettimeofday(&now);

  return timercmp(&now, &_pending_pkthdr.ts, >);
#else
  return 0;
#endif
}

void
FromDump::go() {
  if (!ready()) {
    return;
  }
  output(0).push(_pending_packet);
  _pending_packet = 0;
}

#ifdef HAVE_PCAP
void
FromDump::get_packet(u_char* clientdata,
		       const struct pcap_pkthdr* pkthdr,
		       const u_char* data) {
  FromDump* d = (FromDump*)clientdata;
  d->get_packet(pkthdr, data);
}

void
FromDump::get_packet(const pcap_pkthdr* pkthdr, const u_char* data) {
  // If first time called, set up offset for syncing up real time with
  // the time of the dump.
  if (!timerisset(&_offset)) {
    click_gettimeofday(&_offset);
    _offset.tv_sec -= pkthdr->ts.tv_sec;
    _offset.tv_usec -= pkthdr->ts.tv_usec;
    if (_offset.tv_usec < 0) {
      _offset.tv_usec += 1000000;
      _offset.tv_sec--;
    }
  }

  _pending_packet = Packet::make(data, pkthdr->caplen);
  
  // Fill pkthdr and bump the timestamp by offset
  memcpy(&_pending_pkthdr, pkthdr, sizeof(pcap_pkthdr));
  _pending_pkthdr.ts.tv_sec += _offset.tv_sec;
  _pending_pkthdr.ts.tv_usec += _offset.tv_usec;
  if (_pending_pkthdr.ts.tv_usec > 1000000) {
    _pending_pkthdr.ts.tv_usec -= 1000000;
    _pending_pkthdr.ts.tv_sec++;
  }
}
#endif

EXPORT_ELEMENT(FromDump)
