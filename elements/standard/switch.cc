/*
 * switch.{cc,hh} -- element routes packets to one output of several
 * Eddie Kohler
 *
 * Copyright (c) 2000 Mazu Networks, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "switch.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/llrpc.h>
CLICK_DECLS

Switch::Switch()
{
  add_input();
}

Switch::~Switch()
{
}

void
Switch::notify_noutputs(int n)
{
  set_noutputs(n);
}

int
Switch::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _output = 0;
  if (cp_va_parse(conf, this, errh,
		  cpOptional,
		  cpInteger, "active output", &_output,
		  cpEnd) < 0)
    return -1;
  if (_output >= noutputs())
    _output = -1;
  return 0;
}

void
Switch::configuration(Vector<String> &conf) const
{
  conf.push_back(String(_output));
}

void
Switch::push(int, Packet *p)
{
  checked_output_push(_output, p);
}

String
Switch::read_param(Element *e, void *)
{
  Switch *sw = (Switch *)e;
  return String(sw->_output) + "\n";
}

int
Switch::write_param(const String &in_s, Element *e, void *, ErrorHandler *errh)
{
  Switch *sw = (Switch *)e;
  String s = cp_uncomment(in_s);
  if (!cp_integer(s, &sw->_output))
    return errh->error("Switch output must be integer");
  if (sw->_output >= sw->noutputs())
    sw->_output = -1;
  return 0;
}

void
Switch::add_handlers()
{
  add_read_handler("switch", read_param, (void *)0);
  add_write_handler("switch", write_param, (void *)0);
}

int
Switch::llrpc(unsigned command, void *data)
{
  if (command == CLICK_LLRPC_SET_SWITCH) {
    int32_t *val = reinterpret_cast<int32_t *>(data);
    _output = (*val >= noutputs() ? -1 : *val);
    return 0;

  } else if (command == CLICK_LLRPC_GET_SWITCH) {
    int32_t *val = reinterpret_cast<int32_t *>(data);
    *val = _output;
    return 0;

  } else
    return Element::llrpc(command, data);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Switch)
ELEMENT_MT_SAFE(Switch)
