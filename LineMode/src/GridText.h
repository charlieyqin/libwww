/*                                                     Style Management for Line Mode Browser
                              STYLE DEFINITION FOR HYPERTEXT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module defines the HText functions referenced from the HTML module in the W3C
   Reference Library.
   
 */
#include "WWWLib.h"
#include "HText.h"

extern HTChildAnchor * HText_childNumber (HText * text, int n);

/* delete a hyper doc object with interfering with any anchors */
extern void hyperfree (HText * me);

/* Is there any file left? */
extern BOOL HText_canScrollUp (HText * text);
extern BOOL HText_canScrollDown (HText * text);

/* Move display within window */
extern void HText_scrollUp (HText * text);      /* One page */
extern void HText_scrollDown (HText * text);    /* One page */
extern void HText_scrollTop (HText * text);
extern void HText_scrollBottom (HText * text);

extern int HText_sourceAnchors (HText * text);
extern void HText_setStale (HText * text);
extern void HText_refresh (HText * text);

#ifdef CURSES
extern int HText_getTopOfScreen (HText * text);
extern int HText_getLines (HText * text);
#endif

/*

MEMORY CACHE HANDLER

   Check if document is already loaded. As the application handles the memory cache, we
   call the application to ask. Also check if it has expired in which case we reload it
   (either from disk cache or remotely)
   
 */
extern HTMemoryCacheHandler HTMemoryCache;
/*

   End of declaration  */
