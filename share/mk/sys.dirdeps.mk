# $Id: sys.dirdeps.mk,v 1.9 2023/05/11 20:05:42 sjg Exp $
#
#	@(#) Copyright (c) 2012-2023, Simon J. Gerraty
#
#	This file is provided in the hope that it will
#	be of use.  There is absolutely NO WARRANTY.
#	Permission to copy, redistribute or otherwise
#	use this file is hereby granted provided that
#	the above copyright notice and this notice are
#	left intact.
#
#	Please send copies of changes and bug-fixes to:
#	sjg@crufty.net
#

# Originally DIRDEPS_BUILD and META_MODE were the same thing.
# So, much of this was done in *meta.sys.mk and local*mk
# but properly belongs here.

# Include from [local.]sys.mk - if doing DIRDEPS_BUILD
# we should not be here otherwise
MK_DIRDEPS_BUILD ?= yes
# these are all implied
MK_AUTO_OBJ ?= yes
MK_META_MODE ?= yes
MK_STAGING ?= yes

_PARSEDIR ?= ${.PARSEDIR:tA}

.-include <local.sys.dirdeps.env.mk>

.if ${.MAKE.LEVEL} == 0
# make sure dirdeps target exists and do it first
dirdeps:
# first .MAIN is what counts
.MAIN: dirdeps
.NOPATH: dirdeps
all: dirdeps .WAIT
.endif

.if empty(SRCTOP)
# fallback assumes share/mk!
SRCTOP := ${SB_SRC:U${.PARSEDIR:tA:H:H}}
.export SRCTOP
.endif

# fake SB if not using mk wrapper
.if !defined(SB)
SB := ${SRCTOP:H}
.export SB
.endif

.if empty(OBJROOT)
OBJROOT := ${SB_OBJROOT:U${MAKEOBJDIRPREFIX:U${SB}/obj}/}
.export OBJROOT
.endif

.if empty(STAGE_ROOT)
STAGE_ROOT ?= ${OBJROOT}stage
.export STAGE_ROOT
.endif

# We should be included before meta.sys.mk
# If TARGET_SPEC_VARS is other than just MACHINE
# it should be set by now.
# TARGET_SPEC must not contain any '.'s.
TARGET_SPEC_VARS ?= MACHINE

.if !target(_tspec_env_done_)
_tspec_env_done_: .NOTMAIN

.if ${TARGET_SPEC:Uno:M*,*} != ""
# deal with TARGET_SPEC from env
_tspec := ${TARGET_SPEC:S/,/ /g}
.for i in ${TARGET_SPEC_VARS:${M_RANGE:Urange}}
${TARGET_SPEC_VARS:[$i]} := ${_tspec:[$i]}
.endfor
# We need to stop that TARGET_SPEC affecting any submakes
TARGET_SPEC=
# so export but do not track
.export-env TARGET_SPEC
.export ${TARGET_SPEC_VARS}
.for v in ${TARGET_SPEC_VARS:O:u}
.if empty($v)
.undef $v
.endif
.endfor
.endif
.endif

# Now make sure we know what TARGET_SPEC is
# as we may need it to find Makefile.depend*
.if ${MACHINE:Mhost*} != ""
# host is special
TARGET_SPEC = ${MACHINE}
.else
TARGET_SPEC = ${TARGET_SPEC_VARS:@v@${$v:U}@:ts,}
.endif

.if ${TARGET_SPEC_VARS:[#]} > 1
TARGET_OBJ_SPEC ?= ${TARGET_SPEC_VARS:@v@${$v:U}@:ts.}
.else
TARGET_OBJ_SPEC ?= ${MACHINE}
.endif

MAKE_PRINT_VAR_ON_ERROR += ${TARGET_SPEC_VARS}

.if !defined(MACHINE0)
# it can be handy to know which MACHINE kicked off the build
# for example, if using Makefild.depend for multiple machines,
# allowing only MACHINE0 to update can keep things simple.
MACHINE0 := ${MACHINE}
.export MACHINE0
.endif

.if ${MACHINE} == "host"
OBJTOP = ${HOST_OBJTOP}
.elif ${MACHINE} == "host32"
OBJTOP = ${HOST_OBJTOP32}
.endif

MACHINE_OBJ.host = ${HOST_TARGET}
MACHINE_OBJ.host32 = ${HOST_TARGET32}
MACHINE_OBJ.${MACHINE} ?= ${TARGET_OBJ_SPEC}
MACHINE_OBJDIR = ${MACHINE_OBJ.${MACHINE}}
OBJTOP = ${OBJROOT}/${MACHINE_OBJDIR}

# we do not use MAKEOBJDIRPREFIX
.undef MAKEOBJDIRPREFIX
# we use this
MAKEOBJDIR ?= ${.CURDIR:S,${SRCTOP},${OBJTOP},}

STAGE_MACHINE ?= ${MACHINE_OBJDIR}
STAGE_OBJTOP ?= ${STAGE_ROOT}/${STAGE_MACHINE}
STAGE_COMMON_OBJTOP ?= ${STAGE_ROOT}/common
STAGE_HOST_OBJTOP ?= ${STAGE_ROOT}/${HOST_TARGET}
STAGE_HOST_OBJTOP32 ?= ${STAGE_ROOT}/${HOST_TARGET32}

STAGE_INCLUDEDIR ?= ${STAGE_OBJTOP}${INCLUDEDIR:U/usr/include}
STAGE_LIBDIR ?= ${STAGE_OBJTOP}${LIBDIR:U/lib}

TIME_STAMP_FMT ?= @ %s [%Y-%m-%d %T] ${:U}
DATE_TIME_STAMP ?= `date '+${TIME_STAMP_FMT}'`
TIME_STAMP ?= ${TIME_STAMP_FMT:localtime}

.if ${MK_TIME_STAMPS:Uyes} == "yes"
TRACER = ${TIME_STAMP}
ECHO_DIR = echo ${TIME_STAMP}
ECHO_TRACE = echo ${TIME_STAMP}
.endif

.if ${.CURDIR} == ${SRCTOP}
RELDIR= .
RELTOP= .
.elif ${.CURDIR:M${SRCTOP}/*}
RELDIR:= ${.CURDIR:S,${SRCTOP}/,,}
.else
RELDIR:= ${.OBJDIR:S,${OBJTOP}/,,}
.endif
RELTOP?= ${RELDIR:C,[^/]+,..,g}
RELOBJTOP?= ${RELTOP}
RELSRCTOP?= ${RELTOP}

# this does all the smarts of setting .MAKE.DEPENDFILE
.-include <sys.dependfile.mk>

.-include <local.sys.dirdeps.mk>

# check if we got anything sane
.if ${.MAKE.DEPENDFILE} == ".depend"
.undef .MAKE.DEPENDFILE
.endif
# just in case
.MAKE.DEPENDFILE ?= Makefile.depend

.if ${.MAKE.LEVEL} > 0
# Makefile.depend* also get read at level 1+
# and often refer to DEP_MACHINE etc,
# so ensure DEP_* (for TARGET_SPEC_VARS anyway) are set
.for V in ${TARGET_SPEC_VARS}
DEP_$V = ${$V}
.endfor
.endif
