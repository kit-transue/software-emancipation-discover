(defconst dislite-popup-menu '("DISCOVER" 
						("DISCOVER" 
							("Symbol Info" . 0) 
							("Open Definition" . 1) 
							("Browse Symbol" . 2) 
							("Activate DevX" . 4))))
(defconst dislite-bar-menu
  '("DISCOVER"
     ["Symbol Info" bar-info-proc t]
     ["Open Definition" bar-open-def t]
     ["Browse Symbol" bar-query t]
     ["Activate DevX" bar-browser-popup t]
     )
  "Lemacs style menus for DISCOVER Analysis.")

(defun get-adapter-command ()
	(let (args)
		(setq args
			(append (list (concat (getenv "PSETHOME") "/bin/" "emacsAdapter") "-service" "emacs") args))
	args))

(load (concat (getenv "PSETHOME") "/bin/" "emacsIntegration.el"))
