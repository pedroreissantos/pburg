/* $Id: tabid.h,v 1.3 2019/05/28 15:24:56 prs Exp $ */
/* (C)Pedro Reis dos Santos, 2004-2020 */
#ifndef _TABID_H_
#define _TABID_H_

typedef int (*IDfunc)(int,char*,void*,void*);
void   *IDroot(void *swap);
void	IDpop(void),
	IDpush(void),
	IDclear(void),
	IDprint(int,int);
int	IDnew(int,char*,void*),
	IDreplace(int,char*,void*),
	IDchange(int,char*,void*,int),
	IDinsert(int,int,char*,void*),
	IDfind(char*,void**),
	IDsearch(char*,void**,int,int),
	IDforall(IDfunc,void*,int,int),
	IDlevel(void);

extern int IDdebug; /* set to 1 to get debug message in stdout */

#define IDtest (-1L) /* probe IDfind and IDnew without error messages */

#define IDevery(x,y) IDforall(x,y,0,0)

#endif /* _TABID_H_ */
