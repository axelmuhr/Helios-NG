/*	EFUNC.H:	MicroEMACS function declarations and names

		This file list all the C code functions used by MicroEMACS
	and the names to use to bind keys to them. To add functions,
	declare it here in both the extern function list and the name
	binding table.

*/

/*	Modifications:
	11-Sep-89	Mike Burrow (INMOS)	Added folding.
*/

/*	Name to function binding table

		This table gives the names of all the bindable functions
	end their C function address. These are used for the bind-to-key
	function.
*/

NOSHARE NBIND	names[] = {
	{"abort-command",		ctrlg},
	{"add-global-mode",		setgmode},
	{"add-mode",			setmod},
#if	APROP
	{"apropos",			apro},
#endif
	{"backward-character",		backchar},
	{"begin-macro",			ctlxlp},
	{"beginning-of-file",		gotobob},
	{"beginning-of-line",		gotobol},
	{"bind-to-key",			bindtokey},
	{"buffer-position",		showcpos},
	{"case-region-lower",		lowerregion},
	{"case-region-upper",		upperregion},
	{"case-word-capitalize",	capword},
	{"case-word-lower",		lowerword},
	{"case-word-upper",		upperword},
	{"change-file-name",		filename},
	{"change-screen-size",		newsize},
	{"change-screen-width",		newwidth},
	{"clear-and-redraw",		refresh},
	{"clear-message-line",		clrmes},
	{"close-fold",			closefold},	/* MJB: 11-Sep-89 */
	{"copy-region",			copyregion},
#if	WORDPRO
	{"count-words",			wordcount},
#endif
	{"ctlx-prefix",			cex},
	{"delete-blank-lines",		deblank},
	{"delete-buffer",		killbuffer},
	{"delete-fold",			delfold},	/* MJB: 21-Sep-89 */
	{"delete-global-mode",		delgmode},
	{"delete-mode",			delmode},
	{"delete-next-character",	forwdel},
	{"delete-next-word",		delfword},
	{"delete-other-windows",	onlywind},
	{"delete-previous-character",	backdel},
	{"delete-previous-word",	delbword},
	{"delete-window",		delwind},
	{"describe-bindings",		desbind},
#if	DEBUGM
	{"describe-functions",		desfunc},
#endif
	{"describe-key",		deskey},
#if	DEBUGM
	{"describe-variables",		desvars},
#endif
#if	AEDIT
	{"detab-line",			detab},
	{"detab-region",		detab},
#endif
#if	DEBUGM
	{"display",			dispvar},
#endif
	{"end-macro",			ctlxrp},
	{"end-of-file",			gotoeob},
	{"end-of-line",			gotoeol},
	{"end-of-word",			endword},
#if	AEDIT
	{"entab-line",			entab},
	{"entab-region",		entab},
#endif
	{"enter-fold",			enterfold},	/* MJB: 11-Sep-89 */
	{"exchange-point-and-mark",	swapmark},
	{"execute-buffer",		execbuf},
	{"execute-command-line",	execcmd},
	{"execute-file",		execfile},
	{"execute-macro",		ctlxe},
	{"execute-macro-1",		cbuf1},
	{"execute-macro-10",		cbuf10},
	{"execute-macro-11",		cbuf11},
	{"execute-macro-12",		cbuf12},
	{"execute-macro-13",		cbuf13},
	{"execute-macro-14",		cbuf14},
	{"execute-macro-15",		cbuf15},
	{"execute-macro-16",		cbuf16},
	{"execute-macro-17",		cbuf17},
	{"execute-macro-18",		cbuf18},
	{"execute-macro-19",		cbuf19},
	{"execute-macro-2",		cbuf2},
	{"execute-macro-20",		cbuf20},
	{"execute-macro-21",		cbuf21},
	{"execute-macro-22",		cbuf22},
	{"execute-macro-23",		cbuf23},
	{"execute-macro-24",		cbuf24},
	{"execute-macro-25",		cbuf25},
	{"execute-macro-26",		cbuf26},
	{"execute-macro-27",		cbuf27},
	{"execute-macro-28",		cbuf28},
	{"execute-macro-29",		cbuf29},
	{"execute-macro-3",		cbuf3},
	{"execute-macro-30",		cbuf30},
	{"execute-macro-31",		cbuf31},
	{"execute-macro-32",		cbuf32},
	{"execute-macro-33",		cbuf33},
	{"execute-macro-34",		cbuf34},
	{"execute-macro-35",		cbuf35},
	{"execute-macro-36",		cbuf36},
	{"execute-macro-37",		cbuf37},
	{"execute-macro-38",		cbuf38},
	{"execute-macro-39",		cbuf39},
	{"execute-macro-4",		cbuf4},
	{"execute-macro-40",		cbuf40},
	{"execute-macro-5",		cbuf5},
	{"execute-macro-6",		cbuf6},
	{"execute-macro-7",		cbuf7},
	{"execute-macro-8",		cbuf8},
	{"execute-macro-9",		cbuf9},
	{"execute-named-command",	namedcmd},
#if	PROC
	{"execute-procedure",		execproc},
#endif
	{"execute-program",		execprg},
	{"exit-all-folds",		exitallfolds}, 	/* MJB: 21-Sep-89 */
	{"exit-emacs",			quit},
	{"exit-fold",			exitfold},	/* MJB: 11-Sep-89 */
#if	WORDPRO
	{"fill-paragraph",		fillpara},
#endif
	{"filter-buffer",		filter},
	{"find-file",			filefind},
	{"fold-region",			makefold},	/* MJB: 11-Sep-89 */
	{"forward-character",		forwchar},
	{"goto-line",			gotoline},
	{"goto-mark",			gotomark},
#if	CFENCE
	{"goto-matching-fence",		getfence},
#endif
	{"grow-window",			enlargewind},
	{"handle-tab",			tab},
	{"help",			help},
	{"hunt-backward",		backhunt},
	{"hunt-forward",		forwhunt},
	{"i-shell",			spawncli},
#if	ISRCH
	{"incremental-search",		fisearch},
#endif
	{"insert-file",			insfile},
	{"insert-space",		insspace},
	{"insert-string",		istring},
#if	WORDPRO
	{"kill-paragraph",		killpara},
#endif
	{"kill-region",			killregion},
	{"kill-to-end-of-line",		killtext},
#if	FLABEL
	{"label-function-key",		fnclabel},
#endif
	{"list-buffers",		listbuffers},
	{"macro-to-key",		macrotokey},
	{"meta-prefix",			meta},
#if	MOUSE
	{"mouse-move-down",		movemd},
	{"mouse-move-up",		movemu},
	{"mouse-region-down",		mregdown},
	{"mouse-region-up",		mregup},
	{"mouse-resize-screen",		resizm},
#endif
	{"move-window-down",		mvdnwind},
	{"move-window-up",		mvupwind},
	{"name-buffer",			namebuffer},
	{"narrow-to-region",		narrow},
	{"newline",			newline},
	{"newline-and-indent",		indent},
	{"next-buffer",			nextbuffer},
	{"next-line",			bforwline},
	{"next-page",			forwpage},
#if	WORDPRO
	{"next-paragraph",		gotoeop},
#endif
	{"next-window",			nextwind},
	{"next-word",			forwword},
	{"nop",				nullproc},
	{"open-fold",			openfold},	/* MJB: 11-Sep-89 */
	{"open-line",			openline},
	{"overwrite-string",		ovstring},
	{"pipe-command",		pipecmd},
	{"previous-line",		bbackline},
	{"previous-page",		backpage},
#if	WORDPRO
	{"previous-paragraph",		gotobop},
#endif
	{"previous-window",		prevwind},
	{"previous-word",		backword},
	{"print",			writemsg},
	{"query-replace-string",	qreplace},
	{"quick-exit",			quickexit},
	{"quote-character",		quote},
	{"read-file",			fileread},
	{"redraw-display",		reposition},
	{"remove-fold",			removefold},	/* MJB: 11-Sep-89 */
	{"remove-mark",			remmark},
	{"replace-string",		sreplace},
	{"resize-window",		resize},
	{"restore-window",		restwnd},
#if	ISRCH
	{"reverse-incremental-search",	risearch},
#endif
#if	PROC
	{"run",				execproc},
#endif
	{"save-file",			filesave},
	{"save-window",			savewnd},
	{"scroll-next-down",		nextdown},
	{"scroll-next-up",		nextup},
	{"search-forward",		forwsearch},
	{"search-reverse",		backsearch},
	{"searchfold-forward",		searchffold},	/* MJB: 21-Sep-89 */
	{"searchfold-reverse",		searchbfold},	/* MJB: 21-Sep-89 */
	{"select-buffer",		usebuffer},
	{"set",				setvar},
#if	CRYPT
	{"set-encryption-key",		setekey},
#endif
	{"set-fill-column",		setfillcol},
	{"set-fold-marks",              setfoldmarks},
	{"set-mark",			setmark},
	{"shell-command",		spawn},
	{"shrink-window",		shrinkwind},
	{"source",			execfile},
	{"split-current-window",	splitwind},
	{"store-macro",			storemac},
#if	PROC
	{"store-procedure",		storeproc},
#endif
#if	BSD
	{"suspend-emacs",		bktoshell},
#endif
	{"transpose-characters",	twiddle},
#if	AEDIT
	{"trim-line",			trim},
	{"trim-region",			trim},
#endif
	{"unbind-key",			unbindkey},
	{"universal-argument",		unarg},
	{"unmark-buffer",		unmark},
	{"update-screen",		upscreen},
	{"view-file",			viewfile},
	{"widen-from-region",		widen},
	{"wrap-word",			wrapword},
	{"write-file",			filewrite},
	{"write-message",		writemsg},
	{"yank",			yank},

	{"",			NULL}
};

#define	NFUNCS	(sizeof(names)/sizeof(NBIND)) - 1
