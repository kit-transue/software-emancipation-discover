(if (string-match "xemacs" (downcase (emacs-version)))
    (setq editor-is-xemacs 1)
  (setq editor-is-xemacs nil))

(if (string-match "nt" system-configuration)
    (setq is_unix nil)
  (setq is_unix 1))

(if editor-is-xemacs
    (progn
      (make-face 'dislite-face)
      (set-face-background 'dislite-face "#00dd00")
      (defun dislite-make-selection (start end buffer)
	(progn
	  (setq dislite-selection (make-extent start end buffer))
	  (set-extent-face dislite-selection 'dislite-face)))
      (defun dislite-delete-selection (sel)
	(delete-extent sel))
      (defun dislite-param ()
	(concat "\{" (buffer-file-name) "\} " 
		(number-to-string (extent-start-position dislite-selection)) " " 
		(number-to-string (extent-property dislite-selection 'line)) " "
		(number-to-string (extent-property dislite-selection 'column)) "\n"))
      (defun dislite-emacs-menu (event)
	(interactive "e")
	(popup-menu '("Discover" ["Open definition" dislite-open-def t] ["Browse" dislite-query t] ["Instances" dislite-instances t])))
      (defun dislite-open-def ()
	(interactive)
	(dislite-send (concat "open_def " (dislite-param))))
      (defun dislite-query ()
	(interactive)
	(dislite-send (concat "query " (dislite-param))))
      (defun dislite-instances () 
	(interactive)
	(dislite-send (concat "instances " (dislite-param))))
      (global-unset-key '(shift button1))
      (global-set-key '(shift button1) 'dis-S-mouse-1)
      (global-unset-key '(button3))
      (global-set-key  '(button3) 'dislite-rightclick-cb))
  (setq dislite-face 'region)
  (defun dislite-make-selection (start end buffer)
    (setq dislite-selection (make-overlay start end (current-buffer)))
    (overlay-put dislite-selection 'face dislite-face))
  (defun dislite-delete-selection (sel)
    (delete-overlay sel))
  (defun dislite-param ()
    (concat "\{" (buffer-file-name) "\} " 
	    (number-to-string (overlay-start dislite-selection)) " "
	    (number-to-string (overlay-get dislite-selection 'line)) " "
	    (number-to-string (overlay-get dislite-selection 'column)) "\n"))
  (defun dislite-emacs-menu (substr)
    (setq res (x-popup-menu menu-pos '("Discover" ("Discover" ("Open definition" . 1) ("Browse" . 2) ("Instances" . 3)))))
    (if (numberp res)
	(let* ((param (dislite-param)))
	  (if (= res 1) (dislite-send (concat "open_def " param)))
	  (if (= res 2) (dislite-send (concat "query "    param)))
	  (if (= res 3) (dislite-send (concat "instances " param))))))
  (global-unset-key [S-down-mouse-1])
  (global-set-key [S-mouse-1] 'dis-S-mouse-1)
  (global-set-key [mouse-3] 'dis-ignore-event)
  (global-set-key [down-mouse-3] 'dislite-rightclick-cb))    
  
(dislite-make-selection 1 1 (current-buffer))

(defvar dislite-buffer (generate-new-buffer "*dislite*")
  "*DISCOVER dislite buffer.")

(defvar dislite-current-service (getenv "DIS_SERVICE")
  "*DISCOVER current service name.")

(defvar dislite-process-name "devXpress"
  "*Name of DISCOVER process, that communicates with Wish.")

(defvar dislite-raise-frame nil
  "*DISCOVER flag whether to raise the frame.")

(defvar dislite-cmd-list '(("Emacs_Menu" . dislite-emacs-menu) ("open" . dislite-open-proc) ("openpos" . dislite-openpos-proc) ("openreadonly" . dislite-open-readonly-proc) ("info" . dislite-info-proc) ("Dislite_Error" . dislite-error-proc) ("Dislite_Exit" . dislite-exit) ("Dislite_Restart" . dislite-restart) ("Emacs_Exit" . dislite-emacs-exit) ("Dislite_Service" . dislite-service) ("Dislite_Loaded" . dislite-loaded) ("complete" . di-complete-cb) ("cm_get" . dislite-cm-cb) ("cm_unget" . dislite-cm-cb) ("cm_put" . dislite-cm-cb) ("cm_before_put" . dislite-before-cmput) ("editor_get_reply" . dislite-get-reply-cb))
"*DISCOVER command list.")

(defvar dislite-wakeup-process nil)
(defvar dislite-wakeup-count 0)
(defvar dislite-wakeup-executable nil)

(if (or (not dislite-current-service) (string= dislite-current-service ""))
    (let ((msg "Error: Can't find SERVICE in your preference file. Please delete dislite/dislite.prefs in your home directory\n"))
      (if is_unix
	  (progn
	    ( send-string-to-terminal msg )
	    ( kill-emacs ))
	(setq dislite-current-service ""))))

(defvar dislite-wish (getenv "DIS_WISH")
  "*DISCOVER Wish executable file name.")

(if (not (file-executable-p dislite-wish))
    (let ((msg (concat "Error: Can't find Wish " dislite-wish)))
      ( send-string-to-terminal msg )
      ( beep )
      ( kill-emacs )))

(setq dislite-install (getenv "DIS_INSTALL"))
(setq dislite-client-file (concat dislite-install "/dis_client.tcl"))

(if (not (file-readable-p dislite-client-file))
    (let ((msg (concat "Error: Can't read client file " dislite-client-file)))
      ( send-string-to-terminal msg )
      ( beep )
      ( kill-emacs )))

(defun dislite-report (str)
  (save-excursion
    (set-buffer dislite-buffer)
    (goto-char (point-max))
    (insert str)))

(defun dislite-send (send-str)
  (dislite-report (concat "<- " send-str))
  (process-send-string dislite-process-name send-str))

(defun dislite-start () 
  (message "Loading Discover ...") 
  (start-process dislite-process-name " *dislite*" dislite-wish "-f"  dislite-client-file dislite-current-service )
  (set-process-filter (get-process dislite-process-name) 'dislite-filter)
  (setq title (concat "Discover " dislite-current-service))
  (if editor-is-xemacs
      (setq frame-title-format 'title)
    (modify-frame-parameters (selected-frame) (list ( cons 'name  title)))))

(dislite-start)

(defun dislite-hilite-line ()
  (let ( start end )
    (dislite-delete-selection dislite-selection)
    (end-of-line)
    (setq end (point))
    (beginning-of-line)
    (setq start (point))
    (dislite-make-selection start end (current-buffer))))

(defun dislite-select-token (event)
  (let ( end start line  col str )
    (dislite-delete-selection dislite-selection)
    (mouse-set-point event)
    (skip-syntax-forward "w_")
    (setq end (point))
    (skip-syntax-backward "w_")
    (setq start (point))
    (setq line (count-lines 1 start))
    (setq col (current-column))
    (if (= col 0) (setq line (1+ line)))
    (dislite-make-selection start end (current-buffer))
    (if editor-is-xemacs
	(progn
	  (set-extent-property dislite-selection 'face 'dislite-face)
	  (set-extent-property dislite-selection 'line line)
	  (set-extent-property dislite-selection 'column col))
      (overlay-put dislite-selection 'face dislite-face)  
      (overlay-put dislite-selection 'line   line)
      (overlay-put dislite-selection 'column col))))

(defun dislite-open-proc (substr)
  (let ((pos1 (string-match "{.*}" substr))
	temp-string file line offset)
    (if (numberp pos1)
	(progn
	  (let ((pos2 (match-end 0)))
	    (setq file (substring substr (+ pos1 1) (- pos2 1)))
	    (setq substr (substring substr (+ pos2 1)))))
      (setq temp-string (read-from-string substr))
      (setq file (symbol-name (car temp-string)))
      (setq offset (cdr temp-string))
      (setq substr (substring substr offset)))

    (setq line (read substr)) 
    (find-file file)
    (if (> line 0)
	(progn
	  (goto-line line)
	  (message file)
	  (dislite-hilite-line))
      (if (= line 0)
	  (progn
	    (dislite-delete-selection dislite-selection)
	    (goto-line 1)
	    (message "The symbol may deleted"))
	(goto-line 1)
	(dislite-delete-selection dislite-selection)
	(message file)))
    (setq dislite-raise-frame t)))

(defun dislite-openpos-proc (substr)
  (let ((pos1 (string-match "{.*}" substr))
	pos2 file temp-string offset pos)
    (if (numberp pos1)
	(progn
	  (setq pos2 (match-end 0))
	  (setq file (substring substr (+ pos1 1) (- pos2 1)))
	  (setq substr (substring substr (+ pos2 1))))
    (progn
      (setq temp-string (read-from-string substr))
      (setq file (symbol-name (car temp-string)))
      (setq offset (cdr temp-string))
      (setq substr (substring substr offset))))
    (setq pos (read substr)) 
    (find-file file)
    (if (> pos 0) 
	(progn
	  (goto-char pos)
	  (message file)
	  (hight-line))
      (if (= pos 0)
	  (progn
	    (dislite-delete-selection dislite-selection)
	    (goto-char 1)
	    (message "The symbol may deleted"))
	(progn
	  (goto-char 1)
	  (dislite-delete-selection dislite-selection)
	  (message file)))
      (setq dislite-raise-frame t))))

(defun dislite-open-readonly-proc (substr)
  (dislite-open-proc substr)
  (if (not buffer-read-only)
    (setq buffer-read-only t)))

(defun dislite-info-proc (substr)
  (setq substr (substring substr 2 -1))
  (message "%s" substr)
  (setq dislite-raise-frame t))

(defun dislite-error-proc (substr)
  (setq substr (substring substr 2 -1))
  ( message substr ) 
  (setq dislite-raise-frame t))

(defun dislite-restart (substr)
  (setq dislite-current-service (substring substr 1))
  (modify-frame-parameters 
   (selected-frame) 
   (list ( cons 'name  (concat "Discover " dislite-current-service))))
  
  (kill-process dislite-process-name)
  (setq dislite-wakeup-executable (expand-file-name "wakeup" exec-directory))
  (if (not (file-exists-p dislite-wakeup-executable))
      (setq dislite-wakeup-executable (expand-file-name 
				       "wakeup.exe" (concat (getenv "PSETHOME") "/bin"))))
  (if dislite-wakeup-executable
      (progn
	(message "Discover exited. Restart ...")
	(or (file-directory-p default-directory) (switch-to-buffer "*scratch*"))
	(dislite-wakeup)
	(setq dislite-raise-frame nil))
    (switch-to-buffer "*scratch*")
    (message "Discover exited. Shift click to restart Discover.")))

(defun dislite-exit (substr)
  (setq dislite-current-service (substring substr 1))
  (setq title (concat "Discover " dislite-current-service))
  (modify-frame-parameters (selected-frame) (list ( cons 'name  title)))
  (kill-process dislite-process-name)
  (switch-to-buffer "*scratch*")
  (message "Discover exited. Shift click to restart Discover.")
  (setq dislite-raise-frame nil))

(defun dislite-emacs-exit (substr)
  (if (= (read (substring substr 1)) 1)
      (kill-emacs))
  (setq dislite-raise-frame nil))

(defun dislite-service (substr)
  (let* ((my-substr (substring substr 1))
	 (temp (read my-substr))
	 flag title)
    (if (numberp temp)
	(setq dislite-current-service (number-to-string temp))
      (setq dislite-current-service (symbol-name temp)))
  
    (setq my-substr (substring my-substr (+ (length dislite-current-service) 1)))
    (setq flag (read my-substr))
    
    (setq title (concat "Discover " dislite-current-service))
    (modify-frame-parameters (selected-frame) (list ( cons 'name  title)))
    (setq dislite-raise-frame nil)
    (if (= flag 1)
	(dislite-send (concat "pid_reply" " 0 " "\n")))))
  
(defun dislite-loaded (substr)
  (message "Discover Loaded")
  (setq dislite-raise-frame nil))

(defun dislite-filter (process output)
  (setq str (read-from-string output))
  (let (cmd cmd-proc temp begin end offset substr cmd-con)
    (setq begin 0)
    (setq end (string-match "\n" output))
    (while (progn
	     (setq substr (substring output begin end))
	     (dislite-report (concat "-> " substr "\n"))
	     (setq temp (read-from-string substr))
	     (setq cmd (symbol-name (car temp)))
	     (setq offset (cdr temp))
	     (setq substr (substring substr offset))
	     (if (string= cmd "Error") (send-string-to-terminal substr))
	     (if (and editor-is-xemacs (string= cmd "Emacs_Menu")) 
		       (funcall 'dislite-emacs-menu 'cur_event)
	       (setq cmd-con (assoc cmd dislite-cmd-list))
	       (if cmd-con 
		   (progn
		     (setq cmd-proc (cdr cmd-con))
		     (funcall cmd-proc substr)
		     (if dislite-raise-frame
			 (raise-frame (selected-frame))))))
	     (setq begin (+ end 1))
	     (setq end (string-match "\n" output begin))))))

(defun dis-S-mouse-1 (event)
  (interactive "e")
  (mouse-set-point event)
  (if (get-process dislite-process-name)
      (if (buffer-file-name)
	  (progn
	    (if (buffer-modified-p)
		(progn
		  (beep)
		  (message "Buffer modified. Save it before query."))
	      (dislite-select-token event)
	      (dislite-send (concat "inst_def " (dislite-param))))))
    (dislite-start)))

(defun dis-ignore-event (event)
  (interactive "e"))

(defun dislite-rightclick-cb (event)
  (interactive "e")
  (setq cur_event event)
  (mouse-set-point event)
  (if (not (get-process dislite-process-name))
      (dislite-start)
    (if (buffer-file-name)
	(if (buffer-modified-p)
	    (progn
	      (beep)
	      (message "Buffer modified. Save it before query."))
	  (dislite-select-token event)
	  (if (not editor-is-xemacs)
	      (let* ((pos (event-start event))
		     (posxy (posn-x-y pos)))
		(setq menu-pos (list (list (+ (car posxy) 50) 
					 (+ (cdr posxy) (if is_unix 40 10))) (posn-window pos)))))
	      (dislite-send (concat "right_click " (dislite-param)))))))

(defun dislite-complete (inf)
  (let ((str (concat "complete \{" (buffer-file-name) "\} " (number-to-string (point)) " 0 0 0 " inf "\n") ))
    (dislite-send str)))

(setq after-save-hook '(dislite-save-hook))

(defun dislite-save-hook ()
  (if (get-process dislite-process-name)
      (let* ((filename (buffer-file-name))
	     (str (concat "save_hook \{" filename "\} " 0 "\n")))
	(dislite-send str))))

(setq kill-buffer-hook '(dislite-kill-hook))
(defun dislite-kill-hook ()
  (if (get-process dislite-process-name)
      (let* ((filename (buffer-file-name))
	     (str (concat "kill_hook \{" filename "\} " 0 "\n")))
	(dislite-send str))))

(global-unset-key "\C-x\C-c")
(global-set-key "\C-x\C-c" 'dislite-exit-hook) 

(defun dislite-exit-hook (&optional arg)
  (interactive "P")
  (save-some-buffers arg t)
  (and (or (not (memq t (mapcar (function
				  (lambda (buf) (and (buffer-file-name buf)
						     (buffer-modified-p buf))))
				(buffer-list))))
	   (yes-or-no-p "Modified buffers exist; exit anyway? "))
       (or (not (fboundp 'process-list))
	   ;; process-list is not defined on VMS.
	   (let ((processes (process-list))
		 active)
	     (while processes
	       (and (memq (process-status (car processes)) '(run stop))
		    (let ((val (process-kill-without-query (car processes))))
		      (process-kill-without-query (car processes) val)
		      val)
		    (setq active t))
	       (setq processes (cdr processes)))
	     (or (not active)
		 (yes-or-no-p "Active processes exist; kill them and exit anyway? "))))
       (if (get-process dislite-process-name) 
	   (dislite-send "exit_hook 0 0\n")
	 (kill-emacs))))

(defvar dislite-wakeup-interval 1
  "*Seconds till reconnect to developer Xpress after Model switch.")

(defvar dislite-wakeup-hook nil
  "* List of functions to be called when reconnect to developer Xpress.")

(defun dislite-wakeup ()
  "Reconnect to developer Xpress.
After each reconnect, `dislite-wakeup-hook' is run with `run-hooks'."
  (interactive)
  (setq dislite-wakeup-count 0)
  (let ((live (and dislite-wakeup-process
		   (eq (process-status dislite-wakeup-process) 'run))))
    (if (not live)
	(progn
	  (if dislite-wakeup-process
	      (delete-process dislite-wakeup-process))

	  ;; Using a pty is wasteful, and the separate session causes
	  ;; annoyance sometimes (some systems kill idle sessions).
	  (let ((process-connection-type nil))
	    (setq dislite-wakeup-process
		  (start-process "dislite-wakeup" nil
				 dislite-wakeup-executable
				 (int-to-string dislite-wakeup-interval))))
	  (process-kill-without-query dislite-wakeup-process)
	  (set-process-sentinel dislite-wakeup-process 'dislite-wakeup-sentinel)
	  (set-process-filter dislite-wakeup-process 'dislite-wakeup-filter)))))

(defun dislite-wakeup-sentinel (process event)
  (princ "Launching Developer Xpress ..."))

(defun dislite-wakeup-filter (proc string)
  (if (= dislite-wakeup-count 0)
      (setq dislite-wakeup-count 1)
    (kill-process proc)
    (setq dislite-wakeup-count 0)
    (dislite-start)))

(defun di-string-white (str &optional cur-st cur-en) 
  (let* ((str-len (length str))
         (my-st (if cur-st cur-st 0))
         (my-en (if cur-en cur-en str-len))
         (answer nil))
    (if (and (> str-len 0) (< my-st my-en) (< my-st str-len))
       (while (< my-st my-en)
            (if (null (or (= (aref str my-st) ?   )
			  (= (aref str my-st) ?\t )
			  (= (aref str my-st) ?\n )))
                (setq my-st (+ my-st 1))
            ;else
                (setq answer my-st)
                (setq my-st my-en))))
    answer))

; Returns index of first non space character in a string from cur-st
; to cur-end. Or nil if none.
;
(defun di-string-non-white (str &optional cur-st cur-en)
  (let* ((str-len (length str))
         (my-st (if cur-st cur-st 0))
         (my-en (if cur-en cur-en str-len))
         (answer nil))
    (if (and (> str-len 0) (< my-st my-en) (< my-st str-len))
       (while (< my-st my-en)
            (if (or (= (aref str my-st) ?  )
		    (= (aref str my-st) ?\t)
		    (= (aref str my-st) ?\n))
                (setq my-st (+ my-st 1))
            ;else
                (setq answer my-st)
                (setq my-st my-en))))
    answer))

;Returns alist
(defun di-complete-split (str)
  (let* ((str-len (length str)) (lll nil) (ind 1) (cur-st 0) cur-en)
    (while (< cur-st str-len)
      (let ((st (di-string-non-white str cur-st)) en sub)
	(if (null st)
	    (setq cur-st str-len)
	  (setq en (di-string-white str st))
	  (if (null en) (setq en str-len))
	  (setq cur-st en)
	  (setq sub (substring str st en))
	  (setq lll (cons (cons sub ind) lll))
	  (setq ind (+ ind 1)))))
    lll))

(defun tttest (str)
  (interactive)
  (let* ((lll (di-complete-split str))
	 (ans (completing-read "PROMPT: " lll)))
    ans))

(defun di-complete-apply (str)
  (if (and (stringp str) (> (length str) 0))
      (progn
	(if (or (= (char-syntax (preceding-char)) ?w)
		(= (char-syntax (preceding-char)) ?_))
	    (let ( st )
	      (skip-syntax-backward "w_")
	      (setq st (point))
	      (skip-syntax-forward "w_")
	      (delete-region st (point))))
	(insert str))))

(defun di-complete-cb (substr)
  '(send-string-to-terminal substr)
  (let* ((lst (di-complete-split substr)) ans ttt (killb t))
  '(send-string-to-terminal (concat (prin1-to-string lst) "\n"))
    (if (or (null lst) (= (length lst) 0))
	(message "*** Empty query result.")
      (if (= (length lst) 1)
	  (progn
	    (setq ans (car (car lst)))
	    (message (concat "There is one completion only: " ans)))
	(if di-complete-init
	    (progn
	      (setq ttt (try-completion di-complete-init lst))
	      (if (eq t ttt)
		  (progn
		    (setq ans di-complete-init)
		    (message (concat "Exact completion " di-complete-init)))
		(if (null ttt)
		    (message (concat "No completion for " di-complete-init))
		  (if (eq (try-completion ttt lst) t)
		      (progn
			(setq ans ttt)
			(message (concat "Completed to: " ttt)))
		    (setq killb nil)
		    (di-complete-apply ttt)
		    (setq ans (with-output-to-temp-buffer "*Completions*" 
				(display-completion-list 
				 (all-completions ttt lst))))))))
	  (setq killb nil)
	  (setq ans (with-output-to-temp-buffer "*Completions*" 
		      (display-completion-list (all-completions "" lst)))))))
	  
    '(send-string-to-terminal (concat "di-complete-cb: " ans))
    (if (and (bufferp (get-buffer "*Completions*")) killb) (kill-buffer "*Completions*"))
    (if (stringp ans) (di-complete-apply ans))))


(defun di-back-delimiter (&optional lim)
  (let ((tok nil) (en (point)))
    (if (= (preceding-char) ?.)
	(progn
	  (backward-char 1)
	  (setq tok "."))
      (if (= (preceding-char) ?>)
	  (progn
	    (backward-char 1)
	    (if (= (preceding-char) ?-)
		(progn
		  (backward-char 1)
		  (setq tok "->"))))))
    tok))

(defun di-back-token (&optional lim)
  (let ((tok nil) (en (point)))
    (skip-syntax-backward "w_" lim)
    (if (= (point) en)
	(setq tok nil)
      (setq tok (buffer-substring (point) en)))
    tok))

(defun di-tokenize-back ()
  (let ((lst nil) (st (point)) tok lim (go-on t))
    (beginning-of-line)
    (setq lim (point))
    (goto-char st)
    (while go-on
	(if (or (= (char-syntax (preceding-char)) ?w)
		(= (char-syntax (preceding-char)) ?_))
	    (setq tok (di-back-token lim))
	  (if (or (= (preceding-char) ?.)
		  (= (preceding-char) ?>))
	      (setq tok (di-back-delimiter lim))
	    (if (or (= (char-syntax (preceding-char)) ?\()
		    (= (char-syntax (preceding-char)) ?\)))
		(progn 
		  (setq tok (char-to-string (preceding-char)))
		  (backward-char 1)))))
	(if (or (null tok) (= (point) st))
	    (setq go-on nil)
	  (setq st (point))
	  (setq lst (cons tok lst))))
  lst))
	
(defvar di-complete-tokens nil)
(defvar di-complete-init nil)

(defun di-tokenize-line ()
  (setq di-complete-tokens nil)
  (setq di-complete-init nil)
  (let ((end (point)) lst err-msg)
    (if (or (= (char-syntax (preceding-char)) ?w)
	    (= (char-syntax (preceding-char)) ?_))
	(let ( st )
	  (skip-syntax-backward "w_")
	  (setq st (point))
	  (skip-syntax-forward "w_")
	  (setq di-complete-init (buffer-substring st (point)))
	  '(send-string-to-terminal (concat "di-complete-init: " di-complete-init "\n"))
	  (goto-char st)))
    (if (null (or (= (preceding-char) ?.)
		  (= (preceding-char) ?>)))
	(setq err-msg "The point is not at a class instance or pointer")
      (setq lst (di-tokenize-back)))
    (if err-msg (message err-msg))
    (goto-char end)
    (setq di-complete-tokens lst)))

(defun di-complete-cmd ()
  (interactive)
  (di-tokenize-line)
  (if (> (length di-complete-tokens) 0)
      (let* ((inf (prin1-to-string di-complete-tokens))
	     (len (length inf)))
	(setq inf (substring inf 1 (- len 1)))
	'(send-string-to-terminal (concat "di-complete-cmd: "inf "\n"))
	(message (concat "sending: " inf))
	(dislite-complete inf))))

(defun dislite-cm-cb (substr)
  (let ((pos1 (string-match "{.*}" substr))
	file buf)
    (if (numberp pos1)
	(progn
	  (let ((pos2 (match-end 0)))
	    (setq file (substring substr (+ pos1 1) (- pos2 1)))))
      (setq file (substring substr 1)))
    (if (not is_unix)
	(setq file (file-truename file)))
    (setq buf (get-file-buffer file))
    (if buf
	(save-excursion
	  (set-buffer buf)
	  (revert-buffer nil t)))
    (setq dislite-raise-frame nil)))

(defun dislite-before-cmput (substr)
  (let ((pos1 (string-match "{.*}" substr))
	file buf)
    (if (numberp pos1)
	(progn
	  (let ((pos2 (match-end 0)))
	    (setq file (substring substr (+ pos1 1) (- pos2 1)))))
      (setq file (substring substr 1)))
    (if (not is_unix)
	(setq file (file-truename file)))
    (setq buf (get-file-buffer file))
    (if (and buf (buffer-modified-p buf))
	(progn
	  (switch-to-buffer buf)
	  (dislite-send (concat "cmput_reply \{" file "\} " 1 "\n")))
      (dislite-send (concat "cmput_reply \{" file "\} " 0 "\n"))))
  (setq dislite-raise-frame nil))

(defun dis-get ()
  (interactive ())
  (if (buffer-file-name)
      (dislite-send (concat "editor_get \{" (buffer-file-name) "\} " 0 "\n"))))

(defun dislite-get-reply-cb (substr)
  (dislite-cm-cb substr)
  (message (concat (buffer-file-name) " is checked out.")))



