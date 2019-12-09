/*
 * NatFeats (Native Features main dispatcher)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include <cpu/e68000/e68000.h>
#include <lib/log.h>
#include "atarist.h"
#include "natfeat.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


#define ID_SHIFT	20
#define IDX2MASTERID(idx)	(((idx)+1) << ID_SHIFT)
#define MASTERID2IDX(id)	(((id) >> ID_SHIFT)-1)
#define MASKOUTMASTERID(id)	((id) & ((1L << ID_SHIFT)-1))

#ifdef __EMSCRIPTEN__
#define NAME_STRING "pce-js"
#else
#define NAME_STRING "pce"
#endif
#define VERSION_STRING NAME_STRING " " PCE_VERSION_STR

#if defined __linux__
#  define OS_TYPE "linux"
#elif defined __OpenBSD__
#  define OS_TYPE "openbsd"
#elif defined __FreeBSD__
#  define OS_TYPE "freebsd"
#elif defined __sun
#  define OS_TYPE "solaris"
#elif defined __APPLE__
#  define OS_TYPE "darwin"
#elif defined __CYGWIN32__
#  define OS_TYPE "cywin"
#elif defined __sgi
#  define OS_TYPE "irix"
#elif defined __OS2__
#  define OS_TYPE "os/2"
#elif defined __HAIKU__
#  define OS_TYPE "haiku"
#elif defined __BEOS__
#  define OS_TYPE "beos"
#elif defined __MINT__
#  define OS_TYPE "mint"
#elif defined __MINGW32__
#  define OS_TYPE "mingw"
#elif defined _WIN32
#  define OS_TYPE "mingw"
#elif defined __unix__
#  define OS_TYPE "unix"
#else
#  define OS_TYPE "unknown"
#endif


#if defined(__i386__)
#  define CPU_TYPE "i386"
#elif defined(__x86_64__)
#  define CPU_TYPE "x86_64"
#elif defined(__mc68000__)
#  define CPU_TYPE "m68k"
#elif defined(__arm__)
#  define CPU_TYPE "arm"
#elif defined(__aarch64__)
#  define CPU_TYPE "aarch64"
#else
#  define CPU_TYPE "unknown"
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void atari2HostSafeStrncpy(atari_st_t *sim, char *dest, uint32_t source, size_t count)
{
	while (count > 1 && (*dest = (char) e68_get_mem8(sim->cpu, source)) != 0)
	{
		count--;
		dest++;
		source++;
	}
	if (count > 0)
		*dest = '\0';
}

/*** ---------------------------------------------------------------------- ***/

void host2AtariSafeStrncpy(atari_st_t *sim, uint32_t dest, const char *source, size_t count)
{
	while (count > 1 && *source)
	{
		e68_set_mem8(sim->cpu, dest, *source);
		dest++;
		source++;
		count--;
	}
	if (count > 0)
		e68_set_mem8(sim->cpu, dest, 0);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NF_Name_init(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Name_exit(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Name_reset(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static int32_t NF_Name_dispatch(atari_st_t *sim, uint32_t fncode, uint32_t args)
{
	uint32_t name_ptr = nf_getparameter(sim, args, 0);
	uint32_t name_maxlen = nf_getparameter(sim, args, 1);
	char buffer[1024];
	
	const char *text;

	switch (fncode)
	{
	case 0:								/* get_pure_name(char *name, uint32 max_len) */
		text = NAME_STRING;
		break;

	case 1:								/* get_complete_name(char *name, uint32 max_len) */
		sprintf(buffer, VERSION_STRING " (Host: " OS_TYPE "/" CPU_TYPE "/%s)", sim->trm->name);
		text = buffer;
		break;

	default:
		return -32; /* ENOSYS */
	}

	host2AtariSafeStrncpy(sim, name_ptr, text, name_maxlen);
	return strlen(text);
}

/*** ---------------------------------------------------------------------- ***/

NF_Base const nf_name = {
	NF_Name_init,
	NF_Name_exit,
	NF_Name_reset,
	"NF_NAME",
	0,
	NF_Name_dispatch
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * return version of the NF interface in the form HI.LO (currently 1.0)
 */

static void NF_Version_init(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Version_exit(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Version_reset(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static int32_t NF_Version_dispatch(atari_st_t *sim, uint32_t fncode, uint32_t args)
{
	(void)(args);
	switch (fncode)
	{
	case 0:
		return 0x00010000UL;
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

NF_Base const nf_version = {
	NF_Version_init,
	NF_Version_exit,
	NF_Version_reset,
	"NF_VERSION",
	0,
	NF_Version_dispatch
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NF_Shutdown_init(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Shutdown_exit(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Shutdown_reset(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static int32_t NF_Shutdown_dispatch(atari_st_t *sim, uint32_t fncode, uint32_t args)
{
	switch (fncode)
	{
	case 0:
		pce_log(MSG_INF, "NF_SHUTDOWN(HALT) pc=0x%08x\n", e68_get_mem32(sim->cpu, args - 8));
		trm_set_msg_emu(sim->trm, "emu.exit", "0");
		break;
	case 1:
		pce_log(MSG_INF, "NF_SHUTDOWN(BOOT) pc=0x%08x\n", e68_get_mem32(sim->cpu, args - 8));
		trm_set_msg_emu(sim->trm, "emu.exit", "1");
		break;
	case 2:
		pce_log(MSG_INF, "NF_SHUTDOWN(COLDBOOT) pc=0x%08x\n", e68_get_mem32(sim->cpu, args - 8));
		trm_set_msg_emu(sim->trm, "emu.exit", "2");
		break;
	case 3:
		pce_log(MSG_INF, "NF_SHUTDOWN(POWEROFF) pc=0x%08x\n", e68_get_mem32(sim->cpu, args - 8));
		trm_set_msg_emu(sim->trm, "emu.exit", "3");
		break;
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

NF_Base const nf_shutdown = {
	NF_Shutdown_init,
	NF_Shutdown_exit,
	NF_Shutdown_reset,
	"NF_SHUTDOWN",
	1,
	NF_Shutdown_dispatch
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NF_Exit_init(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Exit_exit(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Exit_reset(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static int32_t NF_Exit_dispatch(atari_st_t *sim, uint32_t fncode, uint32_t args)
{
	uint32_t exitval;
	
	switch (fncode)
	{
	case 0:
		exitval = nf_getparameter(sim, args, 0);
		pce_log(MSG_INF, "NF_EXIT pc=0x%08x\n", e68_get_mem32(sim->cpu, args - 8));
		trm_set_msg_emu(sim->trm, "emu.exit", "0");
		break;
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

NF_Base const nf_exit = {
	NF_Exit_init,
	NF_Exit_exit,
	NF_Exit_reset,
	"NF_EXIT",
	0,
	NF_Exit_dispatch
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * print text on standard error stream
 * internally limited to 2048 characters for now
 */

static void NF_Stderr_init(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Stderr_exit(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void NF_Stderr_reset(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static int32_t NF_Stderr_dispatch(atari_st_t *sim, uint32_t fncode, uint32_t args)
{
	char buffer[2048];
	FILE *output = stderr;
	uint32_t str_ptr;
	int i;
	unsigned short ch;
	
	switch (fncode)
	{
	case 0:
		str_ptr = nf_getparameter(sim, args, 0);
	
		atari2HostSafeStrncpy(sim, buffer, str_ptr, sizeof(buffer));
		for (i = 0; buffer[i] != 0; i++)
		{
			ch = (unsigned char)buffer[i];
			putc(ch, output);
		}
		fflush(output);
		return strlen(buffer);
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

NF_Base const nf_stderr = {
	NF_Stderr_init,
	NF_Stderr_exit,
	NF_Stderr_reset,
	"NF_STDERR",
	0,
	NF_Stderr_dispatch
};


uint32_t nf_get_id(void *ext, uint32_t args)
{
	atari_st_t *sim = (atari_st_t *)ext;
	uint32_t name_ptr = e68_get_mem32(sim->cpu, args);
	char name[1024];
	unsigned int i;
	
	atari2HostSafeStrncpy(sim, name, name_ptr, sizeof(name));

	for (i = 0; i < nf_objs_cnt; i++)
	{
		if (strcasecmp(name, nf_objects[i]->name) == 0)
		{
			return IDX2MASTERID(i);
		}
	}

	return 0;							/* ID with given name not found */
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

const NF_Base *const nf_objects[] = {
	/* NF basic set */
	&nf_name,
	&nf_version,
	&nf_shutdown,
	&nf_exit,
	&nf_stderr,
};
unsigned int const nf_objs_cnt = sizeof(nf_objects) / sizeof(nf_objects[0]);

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

int32_t nf_call(void *ext, uint32_t args)
{
	atari_st_t *sim = (atari_st_t *)ext;
	uint32_t fncode = e68_get_mem32(sim->cpu, args);
	unsigned int idx = MASTERID2IDX(fncode);
	pNatFeat obj;
	
	if (idx >= nf_objs_cnt)
	{
		return 0;						/* returning an undefined value */
	}

	fncode = MASKOUTMASTERID(fncode);
	args += 4;							/* parameters follow on the stack */

	obj = nf_objects[idx];

	if (obj->isSuperOnly && !sim->cpu->supervisor)
	{
		pce_log(MSG_ERR, "nf_call(%s, %d): privilege violation\n", obj->name, fncode);
		e68_exception_privilege(sim->cpu);				/* privilege violation */
		return 0;
	}

	return obj->dispatch(sim, fncode, args);
}
