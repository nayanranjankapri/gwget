/*
 *  Copyright (C) 1999, 2000 Bruno Pires Marinho
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _WGET_LOG_H
#define _WGET_LOG_H

#define BLOCK_SIZE 80

void wget_log_process (GwgetData *gwgetdata);
void wget_drain_remaining_log(GwgetData *gwgetdata);

#endif /* _WGET_LOG_H */
