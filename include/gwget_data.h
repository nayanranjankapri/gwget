/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 

#ifndef _GWGET_DATA_H
#define _GWGET_DATA_H

/* Global Preferences */
typedef struct
{
	gchar *download_dir; /* Default download directory */
	gint num_retries;    /* Number of Retries */
	gboolean resume_at_start;
	gboolean no_create_directories;
	gboolean follow_relative;	/* Follow relative links only */
	gboolean convert_links;		/* Converts to relative links */
	gboolean dl_page_requisites;	/* Download page requisites */
	gint max_depth;
	gboolean view_actual_size;
	gboolean view_total_size;
	gboolean view_percentage;
	gboolean view_elapse_time;
	gboolean view_rem_time;
	gboolean view_down_speed;
	gboolean view_toolbar;
	
} Preferences;

extern Preferences gwget_pref;

typedef enum
{
    DL_NOT_STARTED,       /* We have not started the download of the file */
    DL_NOT_RUNNING,       /* Wget is not running */
    DL_NOT_CONNECTED,     /* Wget is trying to connect with remote host */
    DL_CONNECTED,         /* Wget is connected with remote host */
    DL_RETRIEVING,        /* Wget is retrieving the file */
	DL_COMPLETED,         /* The downloaded is completed */
	DL_ERROR			  /* Error in the download */
} DlState;


/* Struct of the gwget data */
typedef struct 
{
    pid_t wget_pid;             /* Pid of the process running wget */
    gint log_fd;                /* File descriptor of the wget log file */
    gint log_tag;               /* Tag to the function monitoring the log */
    gchar *url;                 /* URL to the file to download */
    gchar *dir;                 /* Directory where the file will be saved */
    gchar *filename;            /* Name of the file being downloaded */
    gchar *local_filename;      /* Used to get the status of the download */
    gboolean use_proxy;         /* Used to use proxy */
    gboolean use_auto_dl;       /* Used to use auto download */
    gchar *line;                /* Used to process the wget output */
    gint line_pos;              /* Used to process the wget output */
    guint32 total_size;         /* Total file size in bytes */
    guint32 total_time;         /* Total time spent in seconds */
    time_t session_start_time;  /* Time at start of this download session */
    guint32 session_start_size; /* Size at start of this download session */
    guint32 session_elapsed;    /* Time spent in seconds on this session */
    guint32 cur_size;           /* Current downloaded file size */
    DlState state;              /* State of the download */
    GtkTreeIter file_list;       /* GtkTreeIter where this file is inserted */
    guint id;                   /* File data id */
    gboolean error;		/* Error in download */
    gchar *error_msg;		/* Error Message */
    gboolean recursive;		/* Recursive download */
	gboolean multimedia;	/* Only download multimedia files in recursive mode */
	gboolean mirror;		/* Mirror the site in recursive mode */
    
} GwgetData;



gint num_of_download;

#define gwget_data_run(gwgetdata) ((gwgetdata->log_tag != -1) ? TRUE : FALSE)

GwgetData * gwget_data_create(gchar *url, gchar *dir);
void gwget_data_start_download(GwgetData *gwgetdata);
void gwget_data_set_state (GwgetData *gwgetdata, DlState state);
void gwget_data_update_statistics (GwgetData *gwgetdata);
void gwget_data_set_total_size (GwgetData *gwgetdata,guint32 total_size);
GwgetData* gwget_data_get_selected(void);
void gwget_data_free(gpointer data);
void gwget_data_stop_download(GwgetData *data);


#endif
