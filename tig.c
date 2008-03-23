/* Copyright (c) 2006-2008 Jonas Fonseca <fonseca@diku.dk>
#define ID_COLS		8
#define NUMBER_INTERVAL	5
#define TIG_BLAME_CMD	""
	REQ_(VIEW_BLAME,	"Show blame view"), \
"   or: tig blame  [rev] path\n"
"  -h, --help      Show help message and exit";
static char opt_file[SIZEOF_STR]	= "";
static char opt_ref[SIZEOF_REF]		= "";
parse_options(int argc, char *argv[])
	size_t buf_size;
	char *subcommand;
	bool seen_dashdash = FALSE;
	int i;
	if (argc <= 1)
		return TRUE;
	subcommand = argv[1];
	if (!strcmp(subcommand, "status") || !strcmp(subcommand, "-S")) {
		opt_request = REQ_VIEW_STATUS;
		if (!strcmp(subcommand, "-S"))
			warn("`-S' has been deprecated; use `tig status' instead");
		if (argc > 2)
			warn("ignoring arguments after `%s'", subcommand);
		return TRUE;
	} else if (!strcmp(subcommand, "blame")) {
		opt_request = REQ_VIEW_BLAME;
		if (argc <= 2 || argc > 4)
			die("invalid number of options to blame\n\n%s", usage);
		i = 2;
		if (argc == 4) {
			string_ncopy(opt_ref, argv[i], strlen(argv[i]));
			i++;
		}
		string_ncopy(opt_file, argv[i], strlen(argv[i]));
		return TRUE;

	} else if (!strcmp(subcommand, "show")) {
		opt_request = REQ_VIEW_DIFF;

	} else if (!strcmp(subcommand, "log") || !strcmp(subcommand, "diff")) {
		opt_request = subcommand[0] == 'l'
			    ? REQ_VIEW_LOG : REQ_VIEW_DIFF;
		warn("`tig %s' has been deprecated", subcommand);
		subcommand = NULL;
	if (!subcommand)
		/* XXX: This is vulnerable to the user overriding
		 * options required for the main view parser. */
		string_copy(opt_cmd, "git log --no-color --pretty=raw --boundary --parents");
	else
		string_format(opt_cmd, "git %s", subcommand);
	buf_size = strlen(opt_cmd);
	for (i = 1 + !!subcommand; i < argc; i++) {
		if (seen_dashdash || !strcmp(opt, "--")) {
			seen_dashdash = TRUE;
		} else if (!strcmp(opt, "-v") || !strcmp(opt, "--version")) {
		} else if (!strcmp(opt, "-h") || !strcmp(opt, "--help")) {
		opt_cmd[buf_size++] = ' ';
		buf_size = sq_quote(opt_cmd, buf_size, opt);
		if (buf_size >= sizeof(opt_cmd))
			die("command too long");
		buf_size = 0;
	opt_cmd[buf_size] = 0;

LINE(STAT_UNTRACKED,"",			COLOR_MAGENTA,	COLOR_DEFAULT,	0), \
LINE(BLAME_DATE,    "",			COLOR_BLUE,	COLOR_DEFAULT,	0), \
LINE(BLAME_AUTHOR,  "",			COLOR_GREEN,	COLOR_DEFAULT,	0), \
LINE(BLAME_COMMIT, "",			COLOR_DEFAULT,	COLOR_DEFAULT,	0), \
LINE(BLAME_ID,     "",			COLOR_MAGENTA,	COLOR_DEFAULT,	0), \
LINE(BLAME_LINENO, "",			COLOR_CYAN,	COLOR_DEFAULT,	0)
	unsigned int dirty:1;
	{ 'B',		REQ_VIEW_BLAME },
	KEYMAP_(BLAME), \
static struct view_ops blame_ops;
	VIEW_(BLAME,  "blame",  &blame_ops,  ref_commit),
draw_text(struct view *view, const char *string, int max_len,
static void
redraw_view_dirty(struct view *view)
{
	bool dirty = FALSE;
	int lineno;

	for (lineno = 0; lineno < view->height; lineno++) {
		struct line *line = &view->line[view->offset + lineno];

		if (!line->dirty)
			continue;
		line->dirty = 0;
		dirty = TRUE;
		if (!draw_view_line(view, lineno))
			break;
	}

	if (!dirty)
		return;
	redrawwin(view->win);
	if (input_mode)
		wnoutrefresh(view->win);
	else
		wrefresh(view->win);
}


	if (view == VIEW(REQ_VIEW_BLAME))
		redraw_view_dirty(view);

	if (view->ops->read(view, NULL))
		end_update(view);
	case REQ_VIEW_BLAME:
		if (!opt_file[0]) {
			report("No file chosen, press %s to open tree view",
			       get_key(REQ_VIEW_TREE));
			break;
		}
		open_view(view, request, OPEN_DEFAULT);
		break;

		   (view == VIEW(REQ_VIEW_DIFF) &&
		     view->parent == VIEW(REQ_VIEW_BLAME)) ||
		draw_text(view, text, view->width, TRUE, tilde_attr);
static char *
tree_path(struct line *line)
{
	char *path = line->data;

	return path + SIZEOF_TREE_ATTR;
}

	if (!text)
		return TRUE;
		char *path1 = tree_path(line);
	if (request == REQ_VIEW_BLAME) {
		char *filename = tree_path(line);

		if (line->type == LINE_TREE_DIR) {
			report("Cannot show blame for directory %s", opt_path);
			return REQ_NONE;
		}

		string_copy(opt_ref, ref_commit);
		string_ncopy(opt_file, filename, strlen(filename));
		return request;
	}
			char *basename = tree_path(line);
	if (!line)
		return TRUE;
/*
 * Blame backend
 *
 * Loading the blame view is a two phase job:
 *
 *  1. File content is read either using opt_file from the
 *     filesystem or using git-cat-file.
 *  2. Then blame information is incrementally added by
 *     reading output from git-blame.
 */

struct blame_commit {
	char id[SIZEOF_REV];		/* SHA1 ID. */
	char title[128];		/* First line of the commit message. */
	char author[75];		/* Author of the commit. */
	struct tm time;			/* Date from the author ident. */
	char filename[128];		/* Name of file. */
};

struct blame {
	struct blame_commit *commit;
	unsigned int header:1;
	char text[1];
};

#define BLAME_CAT_FILE_CMD "git cat-file blob %s:%s"
#define BLAME_INCREMENTAL_CMD "git blame --incremental %s %s"

static bool
blame_open(struct view *view)
{
	char path[SIZEOF_STR];
	char ref[SIZEOF_STR] = "";

	if (sq_quote(path, 0, opt_file) >= sizeof(path))
		return FALSE;

	if (*opt_ref && sq_quote(ref, 0, opt_ref) >= sizeof(ref))
		return FALSE;

	if (*opt_ref) {
		if (!string_format(view->cmd, BLAME_CAT_FILE_CMD, ref, path))
			return FALSE;
	} else {
		view->pipe = fopen(opt_file, "r");
		if (!view->pipe &&
		    !string_format(view->cmd, BLAME_CAT_FILE_CMD, "HEAD", path))
			return FALSE;
	}

	if (!view->pipe)
		view->pipe = popen(view->cmd, "r");
	if (!view->pipe)
		return FALSE;

	if (!string_format(view->cmd, BLAME_INCREMENTAL_CMD, ref, path))
		return FALSE;

	string_format(view->ref, "%s ...", opt_file);
	string_copy_rev(view->vid, opt_file);
	set_nonblocking_input(TRUE);

	if (view->line) {
		int i;

		for (i = 0; i < view->lines; i++)
			free(view->line[i].data);
		free(view->line);
	}

	view->lines = view->line_alloc = view->line_size = view->lineno = 0;
	view->offset = view->lines  = view->lineno = 0;
	view->line = NULL;
	view->start_time = time(NULL);

	return TRUE;
}

static struct blame_commit *
get_blame_commit(struct view *view, const char *id)
{
	size_t i;

	for (i = 0; i < view->lines; i++) {
		struct blame *blame = view->line[i].data;

		if (!blame->commit)
			continue;

		if (!strncmp(blame->commit->id, id, SIZEOF_REV - 1))
			return blame->commit;
	}

	{
		struct blame_commit *commit = calloc(1, sizeof(*commit));

		if (commit)
			string_ncopy(commit->id, id, SIZEOF_REV);
		return commit;
	}
}

static bool
parse_number(char **posref, size_t *number, size_t min, size_t max)
{
	char *pos = *posref;

	*posref = NULL;
	pos = strchr(pos + 1, ' ');
	if (!pos || !isdigit(pos[1]))
		return FALSE;
	*number = atoi(pos + 1);
	if (*number < min || *number > max)
		return FALSE;

	*posref = pos;
	return TRUE;
}

static struct blame_commit *
parse_blame_commit(struct view *view, char *text, int *blamed)
{
	struct blame_commit *commit;
	struct blame *blame;
	char *pos = text + SIZEOF_REV - 1;
	size_t lineno;
	size_t group;

	if (strlen(text) <= SIZEOF_REV || *pos != ' ')
		return NULL;

	if (!parse_number(&pos, &lineno, 1, view->lines) ||
	    !parse_number(&pos, &group, 1, view->lines - lineno + 1))
		return NULL;

	commit = get_blame_commit(view, text);
	if (!commit)
		return NULL;

	*blamed += group;
	while (group--) {
		struct line *line = &view->line[lineno + group - 1];

		blame = line->data;
		blame->commit = commit;
		line->dirty = 1;
	}
	blame->header = 1;

	return commit;
}

static bool
blame_read_file(struct view *view, char *line)
{
	if (!line) {
		FILE *pipe = NULL;

		if (view->lines > 0)
			pipe = popen(view->cmd, "r");
		view->cmd[0] = 0;
		if (!pipe) {
			report("Failed to load blame data");
			return TRUE;
		}

		fclose(view->pipe);
		view->pipe = pipe;
		return FALSE;

	} else {
		size_t linelen = strlen(line);
		struct blame *blame = malloc(sizeof(*blame) + linelen);

		if (!line)
			return FALSE;

		blame->commit = NULL;
		strncpy(blame->text, line, linelen);
		blame->text[linelen] = 0;
		return add_line_data(view, blame, LINE_BLAME_COMMIT) != NULL;
	}
}

static bool
match_blame_header(const char *name, char **line)
{
	size_t namelen = strlen(name);
	bool matched = !strncmp(name, *line, namelen);

	if (matched)
		*line += namelen;

	return matched;
}

static bool
blame_read(struct view *view, char *line)
{
	static struct blame_commit *commit = NULL;
	static int blamed = 0;
	static time_t author_time;

	if (*view->cmd)
		return blame_read_file(view, line);

	if (!line) {
		/* Reset all! */
		commit = NULL;
		blamed = 0;
		string_format(view->ref, "%s", view->vid);
		if (view_is_displayed(view)) {
			update_view_title(view);
			redraw_view_from(view, 0);
		}
		return TRUE;
	}

	if (!commit) {
		commit = parse_blame_commit(view, line, &blamed);
		string_format(view->ref, "%s %2d%%", view->vid,
			      blamed * 100 / view->lines);

	} else if (match_blame_header("author ", &line)) {
		string_ncopy(commit->author, line, strlen(line));

	} else if (match_blame_header("author-time ", &line)) {
		author_time = (time_t) atol(line);

	} else if (match_blame_header("author-tz ", &line)) {
		long tz;

		tz  = ('0' - line[1]) * 60 * 60 * 10;
		tz += ('0' - line[2]) * 60 * 60;
		tz += ('0' - line[3]) * 60;
		tz += ('0' - line[4]) * 60;

		if (line[0] == '-')
			tz = -tz;

		author_time -= tz;
		gmtime_r(&author_time, &commit->time);

	} else if (match_blame_header("summary ", &line)) {
		string_ncopy(commit->title, line, strlen(line));

	} else if (match_blame_header("filename ", &line)) {
		string_ncopy(commit->filename, line, strlen(line));
		commit = NULL;
	}

	return TRUE;
}

static bool
blame_draw(struct view *view, struct line *line, unsigned int lineno, bool selected)
{
	int tilde_attr = -1;
	struct blame *blame = line->data;
	int col = 0;

	wmove(view->win, lineno, 0);

	if (selected) {
		wattrset(view->win, get_line_attr(LINE_CURSOR));
		wchgat(view->win, -1, 0, LINE_CURSOR, NULL);
	} else {
		wattrset(view->win, A_NORMAL);
		tilde_attr = get_line_attr(LINE_MAIN_DELIM);
	}

	if (opt_date) {
		int n;

		if (!selected)
			wattrset(view->win, get_line_attr(LINE_MAIN_DATE));
		if (blame->commit) {
			char buf[DATE_COLS + 1];
			int timelen;

			timelen = strftime(buf, sizeof(buf), DATE_FORMAT, &blame->commit->time);
			n = draw_text(view, buf, view->width - col, FALSE, tilde_attr);
			draw_text(view, " ", view->width - col - n, FALSE, tilde_attr);
		}

		col += DATE_COLS;
		wmove(view->win, lineno, col);
		if (col >= view->width)
			return TRUE;
	}

	if (opt_author) {
		int max = MIN(AUTHOR_COLS - 1, view->width - col);

		if (!selected)
			wattrset(view->win, get_line_attr(LINE_MAIN_AUTHOR));
		if (blame->commit)
			draw_text(view, blame->commit->author, max, TRUE, tilde_attr);
		col += AUTHOR_COLS;
		if (col >= view->width)
			return TRUE;
		wmove(view->win, lineno, col);
	}

	{
		int max = MIN(ID_COLS - 1, view->width - col);

		if (!selected)
			wattrset(view->win, get_line_attr(LINE_BLAME_ID));
		if (blame->commit)
			draw_text(view, blame->commit->id, max, FALSE, -1);
		col += ID_COLS;
		if (col >= view->width)
			return TRUE;
		wmove(view->win, lineno, col);
	}

	{
		unsigned long real_lineno = view->offset + lineno + 1;
		char number[10] = "          ";
		int max = MIN(view->digits, STRING_SIZE(number));
		bool showtrimmed = FALSE;

		if (real_lineno == 1 ||
		    (real_lineno % opt_num_interval) == 0) {
			char fmt[] = "%1ld";

			if (view->digits <= 9)
				fmt[1] = '0' + view->digits;

			if (!string_format(number, fmt, real_lineno))
				number[0] = 0;
			showtrimmed = TRUE;
		}

		if (max > view->width - col)
			max = view->width - col;
		if (!selected)
			wattrset(view->win, get_line_attr(LINE_BLAME_LINENO));
		col += draw_text(view, number, max, showtrimmed, tilde_attr);
		if (col >= view->width)
			return TRUE;
	}

	if (!selected)
		wattrset(view->win, A_NORMAL);

	if (col >= view->width)
		return TRUE;
	waddch(view->win, ACS_VLINE);
	col++;
	if (col >= view->width)
		return TRUE;
	waddch(view->win, ' ');
	col++;
	col += draw_text(view, blame->text, view->width - col, TRUE, tilde_attr);

	return TRUE;
}

static enum request
blame_request(struct view *view, enum request request, struct line *line)
{
	enum open_flags flags = display[0] == view ? OPEN_SPLIT : OPEN_DEFAULT;
	struct blame *blame = line->data;

	switch (request) {
	case REQ_ENTER:
		if (!blame->commit) {
			report("No commit loaded yet");
			break;
		}

		if (!strcmp(blame->commit->id, "0000000000000000000000000000000000000000")) {
			char path[SIZEOF_STR];

			if (sq_quote(path, 0, view->vid) >= sizeof(path))
				break;
			string_format(opt_cmd, "git diff-index --root --patch-with-stat -C -M --cached HEAD -- %s 2>/dev/null", path);
		}

		open_view(view, REQ_VIEW_DIFF, flags);
		break;

	default:
		return request;
	}

	return REQ_NONE;
}

static bool
blame_grep(struct view *view, struct line *line)
{
	struct blame *blame = line->data;
	struct blame_commit *commit = blame->commit;
	regmatch_t pmatch;

#define MATCH(text) \
	(*text && regexec(view->regex, text, 1, &pmatch, 0) != REG_NOMATCH)

	if (commit) {
		char buf[DATE_COLS + 1];

		if (MATCH(commit->title) ||
		    MATCH(commit->author) ||
		    MATCH(commit->id))
			return TRUE;

		if (strftime(buf, sizeof(buf), DATE_FORMAT, &commit->time) &&
		    MATCH(buf))
			return TRUE;
	}

	return MATCH(blame->text);

#undef MATCH
}

static void
blame_select(struct view *view, struct line *line)
{
	struct blame *blame = line->data;
	struct blame_commit *commit = blame->commit;

	if (!commit)
		return;

	if (!strcmp(commit->id, "0000000000000000000000000000000000000000"))
		string_ncopy(ref_commit, "HEAD", 4);
	else
		string_copy_rev(ref_commit, commit->id);
}

static struct view_ops blame_ops = {
	"line",
	blame_open,
	blame_read,
	blame_draw,
	blame_request,
	blame_grep,
	blame_select,
};
		draw_text(view, text, view->width, TRUE, tilde_attr);
	draw_text(view, status->new.name, view->width - 5, TRUE, tilde_attr);
	case REQ_VIEW_BLAME:
		if (status) {
			string_copy(opt_file, status->new.name);
			opt_ref[0] = 0;
		}
		return request;

	case REQ_VIEW_BLAME:
		if (stage_status.new.name[0]) {
			string_copy(opt_file, stage_status.new.name);
			opt_ref[0] = 0;
		}
		return request;

		n = draw_text(view, buf, view->width - col, FALSE, tilde_attr);
		draw_text(view, " ", view->width - col - n, FALSE, tilde_attr);
		draw_text(view, commit->author, max_len, TRUE, tilde_attr);
			col += draw_text(view, "[", view->width - col, TRUE, tilde_attr);
			col += draw_text(view, commit->refs[i]->name, view->width - col,
					 TRUE, tilde_attr);
			col += draw_text(view, "]", view->width - col, TRUE, tilde_attr);
			col += draw_text(view, " ", view->width - col, TRUE, tilde_attr);
	draw_text(view, commit->title, view->width - col, TRUE, tilde_attr);