#ifndef strtools_h
#define strtools_h

#define cmp_str_elptr(N, L, E) __cmp_str_elptr(N, L, (TH_element *)(E))

uint16_t fstr_len(const __FlashStringHelper *ifsh);
uint8_t cmp_str_flash(char *str, const __FlashStringHelper *flsh);
uint8_t cmp_flash_flash(const __FlashStringHelper *flsh1, const __FlashStringHelper *flsh2);
uint8_t __cmp_str_elptr(char *name, uint16_t name_len, TH_element *el);
uint8_t cmp_flhp_elptr(const __FlashStringHelper *flph, uint8_t len, TH_element *el);




#endif

