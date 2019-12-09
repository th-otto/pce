#ifndef _NF_NATFEAT_H
#define _NF_NATFEAT_H

typedef struct _nf_base NF_Base;
struct _nf_base
{
	void (*init)(void);
	void (*exit)(void);
	void (*reset)(void);
	const char *name;
	int isSuperOnly;
	int32_t (*dispatch)(atari_st_t *sim, uint32_t fncode, uint32_t args);
};

typedef const NF_Base *pNatFeat;

extern const NF_Base *const nf_objects[];
extern unsigned int const nf_objs_cnt;

extern void NFReset(void);
extern void NFCreate(void);
extern void NFDestroy(void);

uint32_t nf_get_id(void *ext, uint32_t args);
int32_t nf_call(void *ext, uint32_t args);


static uint32_t nf_getparameter(atari_st_t *sim, uint32_t args, int i)
{
	if (i < 0)
		return 0;

	return e68_get_mem32(sim->cpu, args + i * 4);
}

static void Atari2Host_memcpy(atari_st_t *sim, void *_dst, uint32_t _src, size_t count)
{
	const void *src = mem_get_ptr(sim->mem, _src, count);
	memcpy(_dst, src, count);
}

static void Host2Atari_memcpy(atari_st_t *sim, uint32_t _dst, const void *_src, size_t count)
{
	void *dst = mem_get_ptr(sim->mem, _dst, count);
	memcpy(dst, _src, count);
}

size_t atari2HostSafeStrlen(atari_st_t *sim, uint32_t source);
void host2AtariSafeStrncpy(atari_st_t *sim, uint32_t dest, const char *source, size_t count);
void atari2HostSafeStrncpy(atari_st_t *sim, char *dest, uint32_t source, size_t count);

#endif
