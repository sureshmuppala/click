// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * striptonet.{cc,hh} -- element strips to network header
 * Eddie Kohler
 *
 * Copyright (c) 2001 International Computer Science Institute
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
#include "striptonet.hh"
#include <click/error.hh>
#include <click/glue.hh>
CLICK_DECLS

StripToNetworkHeader::StripToNetworkHeader()
    : Element(1, 1)
{
}

StripToNetworkHeader::~StripToNetworkHeader()
{
}

Packet *
StripToNetworkHeader::simple_action(Packet *p)
{
    int off = p->network_header_offset();
    if (off >= 0) {
	p->pull(off);
	return p;
    } else
	return p->nonunique_push(-off);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(StripToNetworkHeader)
ELEMENT_MT_SAFE(StripToNetworkHeader)
