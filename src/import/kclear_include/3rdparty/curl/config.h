#ifndef __CURL_CONFIG_H
#define __CURL_CONFIG_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2005, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: multi.h,v 1.25 2005/04/18 11:40:50 bagder Exp $
 ***************************************************************************/

#if defined(_WIN32) && !defined(WIN32)
/* Chris Lewis mentioned that he doesn't get WIN32 defined, only _WIN32 so we
   make this adjustment to catch this. */
#define WIN32 1
#endif

/* Define this if this file was updated by configure */
#undef CURL_CONFIGURE

#ifndef CURL_CONFIGURE
/* HP-UX systems version 9, 10 and 11 lack sys/select.h and so does oldish
   libc5-based Linux systems. Only include it on system that are known to
   require it! */
#if defined(_AIX) || defined(NETWARE)
#define CURL_HAVE_SYS_SELECT_H
#endif

#else /* CURL_CONFIGURE */

/* Define with if sys/select.h is present */
#undef CURL_HAVE_SYS_SELECT_H

/* The size of a `curl_off_t', as computed by sizeof. */
#define CURL_SIZEOF_CURL_OFF_T 0

#endif /* CURL_CONFIGURE */

#endif /* __CURL_CONFIG_H */
