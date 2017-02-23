/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef SPELL_H
#define SPELL_H

/*
  TODO stuff we need to do for this spell module:

  eliminate all the stderr fprintfs
*/

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _sp_suggestions {
	int count;
	short *score;
	unsigned short **word;
} sp_suggestions;
   
int SpellCheckInit(char *hashname);
void SpellCheckCleanup(void);
int SpellCheckNWord16(const unsigned short *word16, int length);
int SpellCheckSuggestNWord16(const unsigned short *word16, int length, sp_suggestions *sg);

#ifdef __cplusplus
}
#endif
	
#endif /* SPELL_H */
