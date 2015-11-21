(defconst dislite-popup-menu '("DIScover Worktray" 
						("DIScover Worktray" 
							("Symbol Info" . 0) 
							("Open Definition" . 1) 
							("Browse Symbol" . 2) 
							("Activate Browser" . 4))))
(defconst dislite-bar-menu
  '("DIScover Worktray"
     ["Symbol Info" bar-info-proc t]
     ["Open Definition" bar-open-def t]
     ["Browse Symbol" bar-query t]
     ["Activate Browser" bar-browser-popup t]
     )
  "Lemacs style menus for DIScover.")

(defun get-adapter-command ()
	(let (args)
		(setq args
			(append (list (concat (getenv "PSETHOME") "/bin/" "editorServer") "EmacsIntegrator") args))
	args))

(setenv "DIS_WISH" (concat (getenv "PSETHOME") "/bin/" "wish83"))

(load (concat (getenv "PSETHOME") "/bin/" "emacsIntegration.el"))
