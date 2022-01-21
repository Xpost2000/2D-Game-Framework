;; This is an emacs helper I guess
;; will most definitely not work without my config so this is really just for me.
;; it's just convenient to have this around I guess
;; remember to M-X load-file "helper.el"

;; I don't even really use this at this point lol.
(defun grep-for (comment-type)
  (grep (format "grep -n %s *.c *.h" comment-type)))

(definteractive grep-for-todo () (grep-for "TODO"))
(definteractive grep-for-note () (grep-for "NOTE"))

(defhydra project-hydra (global-map "<f4>")
  ("t" #'grep-for-todo)
  ("r" #'grep-for-note))
