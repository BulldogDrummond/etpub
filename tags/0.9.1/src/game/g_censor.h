#ifndef _G_CENSOR_H
#define _G_CENSOR_H

void G_CensorPenalize(gentity_t *ent);
qboolean G_CensorText(char *text, wordDictionary *dictionary);
qboolean G_CensorName(char *name, char *userinfo, int clientNum);


#endif /* ifndef _G_CENSOR_H */
