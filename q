RUN-MAILCAP(1)                       Run Mailcap Programs                       RUN-MAILCAP(1)

NNAAMMEE
       run-mailcap,  view,  see,  edit,  compose,  print - execute programs via entries in the
       _m_a_i_l_c_a_p file

SSYYNNOOPPSSIISS
       rruunn--mmaaiillccaapp --action=ACTION [--option[=value]] [MIME-TYPE:[ENCODING:]]FILE [...]

       The sseeee, eeddiitt, ccoommppoossee and pprriinntt versions are just aliases that default  to  the  view,
       edit, compose, and print actions (respectively).

DDEESSCCRRIIPPTTIIOONN
       rruunn--mmaaiillccaapp  (or  any  of  its aliases) will use the given action to process each mime-
       type/file in turn.  Each file is specified as its mime-type, its  encoding  (e.g.  com‐
       pression), and filename together, separated by colons.  If the mime-type is omitted, an
       attempt to determine the type is made by trying to  match  the  file's  extension  with
       those  in  the _m_i_m_e_._t_y_p_e_s files.  If no mime-type is found, a last attempt will be done
       by running the ffiillee command, if available.  If the encoding is omitted, it will also be
       determined  from  the file's extensions.  Currently supported encodings are ggzziipp (.gz),
       bbzziipp22 (.bz2), xxzz (.xz), and ccoommpprreessss (.Z).  A filename of  "-"  can  be  used  to  mean
       "standard input", but then a mime-type mmuusstt be specified.

       Both  the  user's files (~/.mailcap; ~/.mime.types) and the system files (/etc/mailcap;
       /etc/mime.types) are searched in turn for information.

   EEXXAAMMPPLLEESS
         see picture.jpg
         print output.ps.gz
         compose text/html:index.htm
         extract-mail-attachment msg.txt | see image/tiff:gzip:-

   OOPPTTIIOONNSS
       All options are in the form --<opt>=<value>.

       ----aaccttiioonn==<<aaccttiioonn>>
              Performs the specified action on the files.  Valid actions are _v_i_e_w,  _c_a_t  (uses
              only  "copiousoutput" rules and sends output to STDOUT) , _c_o_m_p_o_s_e, _c_o_m_p_o_s_e_t_y_p_e_d,
              _e_d_i_t and _p_r_i_n_t.  If no action is specified, the action will be determined by how
              the program was called.

       ----ddeebbuugg
              Turns on extra information to find out what is happening.

       ----nnooppaaggeerr
              Ignores any "copiousoutput" directive and sends output to STDOUT.

       ----nnoorruunn
              Displays the found command without actually executing it.

SSEECCUURRIITTYY
       A temporary symbolic link to the file is opened if the file name matches the Perl regu‐
       lar expression "[^[:alnum:],.:/@%^+=_-]", in order to protect  from  the  injection  of
       shell  commands,  and to make sure that the name can always be displayed in the current
       locale.  In addition, the file is opened using its absolute path to prevent the  injec‐
       tion of command-line arguments, for instance using file names starting with dashes.

SSEEEE AALLSSOO
       ffiillee(1) mmaaiillccaapp(5) mmaaiillccaapp..oorrddeerr(5) uuppddaattee--mmiimmee(8)

AAUUTTHHOORR
       rruunn--mmaaiillccaapp (and its aliases) was written by Brian White <bcwhite@pobox.com>.

CCOOPPYYRRIIGGHHTT
       rruunn--mmaaiillccaapp (and its aliases) is in the public domain (the only true "free").

Debian Project                           1st Jan 2008                           RUN-MAILCAP(1)
