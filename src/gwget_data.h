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
	gboolean ask_save_each_dl; 
	gint num_retries;    /* Number of Retries */
	gchar *http_proxy; /* HTTP Proxy */
	gint http_proxy_port; /* HTTP Proxy Port */
	gchar *proxy_user;	/*Proxy user*/
	gchar *proxy_password; /*Proxy password*/
	gboolean proxy_uses_auth; /*Proxy requires password*/
	gchar *gnome_http_proxy; /* GNOME HTTP Proxy */
	gint gnome_http_proxy_port; /* GNOME HTTP Proxy Port */
	gchar *gnome_proxy_user;	/*GNOME Proxy user*/
	gchar *gnome_proxy_password; /*GNOME Proxy password*/
	gboolean gnome_use_proxy;    
	gboolean gnome_proxy_uses_auth; /*GNOME Proxy requires password*/
	gchar *network_mode;
	gboolean resume_at_start;
	gboolean open_after_dl; /* Open the file after Download */
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
	gboolean view_statusbar;
	gboolean view_file_type;
	gboolean docked;
	gboolean limit_speed;
	gboolean limit_simultaneousdownloads;
	gint max_speed;
	gint max_simultaneousdownloads;
	gboolean trayonly;
} Preferences;

Preferences gwget_pref;

typedef enum
{
	DL_NOT_STARTED,		/* We have not started the download of the file */
	DL_NOT_RUNNING,		/* Wget is not running */
	DL_NOT_CONNECTED,	/* Wget is trying to connect with remote host */
	DL_CONNECTED,		/* Wget is connected with remote host */
	DL_RETRIEVING,		/* Wget is retrieving the file */
	DL_COMPLETED,         	/* The downloaded is completed */
	DL_ERROR,		/* Error in the download */
	DL_WAITING		/* Waiting for another try */
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
    guint32 line_blocks;
    guint32 line_pos;
    guint64 total_size;         /* Total file size in bytes */
    guint32 total_time;         /* Total time spent in seconds */
    time_t session_start_time;  /* Time at start of this download session */
    guint64 session_start_size; /* Size at start of this download session */
    guint32 session_elapsed;    /* Time spent in seconds on this session */
    guint64 cur_size;           /* Current downloaded file size */
    DlState state;              /* State of the download */
    GtkTreeIter file_list;       /* GtkTreeIter where this file is inserted */
    guint id;                   /* File data id */
    gboolean error;		/* Error in download */
    gchar *error_msg;		/* Error Message */
    gboolean recursive;		/* Recursive download */
	gboolean multimedia;	/* Only download multimedia files in recursive mode */
	gboolean mirror;		/* Mirror the site in recursive mode */
	gchar *state_str;
   gchar *icon_name; 
} GwgetData;



gint num_of_download;

#define gwget_data_run(gwgetdata) ((gwgetdata->log_tag != -1) ? TRUE : FALSE)

GwgetData * gwget_data_create (gchar *url, gchar *dir);
GwgetData * gwget_data_new (gchar *url);
void gwget_data_set_filename(GwgetData* gwgetdata,gchar *str);
void gwget_data_start_download(GwgetData *gwgetdata);
void gwget_data_set_state (GwgetData *gwgetdata, DlState state);
void gwget_data_set_state_no_sync (GwgetData *gwgetdata, DlState state);
void gwget_data_update_statistics (GwgetData *gwgetdata);
void gwget_data_set_total_size (GwgetData *gwgetdata,guint64 total_size);
GwgetData* gwget_data_get_selected(void);
void gwget_data_free(gpointer data);
void gwget_data_stop_download(GwgetData *data);
void gwget_data_set_filename_from_url(GwgetData *gwgetdata, gchar *url);
void gwget_data_add_download(GwgetData *gwgetdata);
void gwget_data_exec(GwgetData *gwgetdata);
void gwget_data_set_menus (GwgetData *gwgetdata);
void gwget_data_set_popupmenu (GwgetData *gwgetdata);

void gwget_init_pref(Preferences *pref);

#endif
