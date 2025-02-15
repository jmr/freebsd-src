/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1997 Peter Wemm.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the FreeBSD Project
 *	by Peter Wemm.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * so there!
 *
 * $FreeBSD$
 */

#pragma once

#include <libifconfig.h>
#include <stdbool.h>

#define	__constructor	__attribute__((constructor))

struct afswtch;
struct cmd;

typedef	void c_func(const char *cmd, int arg, int s, const struct afswtch *afp);
typedef	void c_func2(const char *arg1, const char *arg2, int s,
    const struct afswtch *afp);
typedef	void c_func3(const char *cmd, const char *arg, int s,
    const struct afswtch *afp);

struct cmd {
	const char *c_name;
	int	c_parameter;
#define	NEXTARG		0xffffff	/* has following arg */
#define	NEXTARG2	0xfffffe	/* has 2 following args */
#define	OPTARG		0xfffffd	/* has optional following arg */
#define	SPARAM		0xfffffc	/* parameter is string c_sparameter */
	const char *c_sparameter;
	union {
		c_func	*c_func;
		c_func2	*c_func2;
		c_func3	*c_func3;
	} c_u;
	int	c_iscloneop;
	struct cmd *c_next;
};
void	cmd_register(struct cmd *);

typedef	void callback_func(int s, void *);
void	callback_register(callback_func *, void *);

/*
 * Macros for declaring command functions and initializing entries.
 */
#define	DECL_CMD_FUNC(name, cmd, arg) \
	void name(const char *cmd, int arg, int s, const struct afswtch *afp)
#define	DECL_CMD_FUNC2(name, arg1, arg2) \
	void name(const char *arg1, const char *arg2, int s, \
	    const struct afswtch *afp)

#define	DEF_CMD(name, param, func) {		\
    .c_name = (name),				\
    .c_parameter = (param),			\
    .c_u = { .c_func = (func) },		\
    .c_iscloneop = 0,				\
    .c_next = NULL,				\
}
#define	DEF_CMD_ARG(name, func) {		\
    .c_name = (name),				\
    .c_parameter = NEXTARG,			\
    .c_u = { .c_func = (func) },		\
    .c_iscloneop = 0,				\
    .c_next = NULL,				\
}
#define	DEF_CMD_OPTARG(name, func) {		\
    .c_name = (name),				\
    .c_parameter = OPTARG,			\
    .c_u = { .c_func = (func) },		\
    .c_iscloneop = 0,				\
    .c_next = NULL,				\
}
#define	DEF_CMD_ARG2(name, func) {		\
    .c_name = (name),				\
    .c_parameter = NEXTARG2,			\
    .c_u = { .c_func2 = (func) },		\
    .c_iscloneop = 0,				\
    .c_next = NULL,				\
}
#define	DEF_CMD_SARG(name, sparam, func) {	\
    .c_name = (name),				\
    .c_parameter = SPARAM,			\
    .c_sparameter = (sparam),			\
    .c_u = { .c_func3 = (func) },		\
    .c_iscloneop = 0,				\
    .c_next = NULL,				\
}
#define	DEF_CLONE_CMD(name, param, func) {	\
    .c_name = (name),				\
    .c_parameter = (param),			\
    .c_u = { .c_func = (func) },		\
    .c_iscloneop = 1,				\
    .c_next = NULL,				\
}
#define	DEF_CLONE_CMD_ARG(name, func) {		\
    .c_name = (name),				\
    .c_parameter = NEXTARG,			\
    .c_u = { .c_func = (func) },		\
    .c_iscloneop = 1,				\
    .c_next = NULL,				\
}
#define	DEF_CLONE_CMD_ARG2(name, func) {	\
    .c_name = (name),				\
    .c_parameter = NEXTARG2,			\
    .c_u = { .c_func2 = (func) },		\
    .c_iscloneop = 1,				\
    .c_next = NULL,				\
}

struct ifaddrs;
struct addrinfo;

enum {
	RIDADDR,
	ADDR,
	MASK,
	DSTADDR,
};

struct afswtch {
	const char	*af_name;	/* as given on cmd line, e.g. "inet" */
	short		af_af;		/* AF_* */
	/*
	 * Status is handled one of two ways; if there is an
	 * address associated with the interface then the
	 * associated address family af_status method is invoked
	 * with the appropriate addressin info.  Otherwise, if
	 * all possible info is to be displayed and af_other_status
	 * is defined then it is invoked after all address status
	 * is presented.
	 */
	void		(*af_status)(int, const struct ifaddrs *);
	void		(*af_other_status)(int);
					/* parse address method */
	void		(*af_getaddr)(const char *, int);
					/* parse prefix method (IPv6) */
	void		(*af_getprefix)(const char *, int);
	void		(*af_postproc)(int s, const struct afswtch *,
			    int newaddr, int ifflags);
	u_long		af_difaddr;	/* set dst if address ioctl */
	u_long		af_aifaddr;	/* set if address ioctl */
	void		*af_ridreq;	/* */
	void		*af_addreq;	/* */
	struct afswtch	*af_next;

	/* XXX doesn't fit model */
	void		(*af_status_tunnel)(int);
	void		(*af_settunnel)(int s, struct addrinfo *srcres,
				struct addrinfo *dstres);
};
void	af_register(struct afswtch *);

struct ifconfig_args {
	bool all;		/* Match everything */
	bool downonly;		/* Down-only items */
	bool uponly;		/* Up-only items */
	bool namesonly;		/* Output only names */
	bool noload;		/* Do not load relevant kernel modules */
	bool supmedia;		/* Supported media */
	bool printkeys;		/* Print security keys */
	int verbose;		/* verbosity level */
	int argc;
	char **argv;
	const char *ifname;	/* Requested interface name */
	const char *matchgroup;		/* Group name to match */
	const char *nogroup;		/* Group name to exclude */
	const struct afswtch *afp;	/* AF we're operating on */
};

struct option {
	const char *opt;
	const char *opt_usage;
	void	(*cb)(const char *arg);
	struct option *next;
};
void	opt_register(struct option *);

extern	ifconfig_handle_t *lifh;
extern	struct ifreq ifr;
extern	char name[IFNAMSIZ];	/* name of interface */
extern	int allmedia;
extern	int printkeys;
extern	int newaddr;
extern	int verbose;
extern	int printifname;
extern	int exit_code;
extern struct ifconfig_args args;

void	setifcap(const char *, int value, int s, const struct afswtch *);
void	setifcapnv(const char *vname, const char *arg, int s,
	    const struct afswtch *afp);

void	Perror(const char *cmd);
void	printb(const char *s, unsigned value, const char *bits);

void	ifmaybeload(struct ifconfig_args *args, const char *name);

typedef int  clone_match_func(const char *);
typedef void clone_callback_func(int, struct ifreq *);
void	clone_setdefcallback_prefix(const char *, clone_callback_func *);
void	clone_setdefcallback_filter(clone_match_func *, clone_callback_func *);

void	sfp_status(int s, struct ifreq *ifr, int verbose);

/*
 * XXX expose this so modules that neeed to know of any pending
 * operations on ifmedia can avoid cmd line ordering confusion.
 */
struct ifmediareq *ifmedia_getstate(void);

void print_vhid(const struct ifaddrs *, const char *);

void ioctl_ifcreate(int s, struct ifreq *);
