//
//  fancyOut.h
//  OSTN02
//
//  Created by George MacKerron on 20/02/2012.
//  Copyright (c) 2012 George MacKerron. All rights reserved.
//

#ifndef OSTN02_fancyOut_h
#define OSTN02_fancyOut_h

#ifdef UNFANCY_OUTPUT
#define BOLD      ""
#define UNBOLD    ""
#define INVERSE   ""
#define UNINVERSE ""
#define ULINE     ""
#define UNULINE   ""
#else
#define BOLD      "\033[1m"
#define UNBOLD    "\033[22m"
#define INVERSE   "\033[7m"
#define UNINVERSE "\033[27m"
#define ULINE     "\033[4m"
#define UNULINE   "\033[24m"
#endif

#endif
