; Файл с макросом для создания слов в словаре

%define first 0

%macro colon 2o
%ifnstr %1
    %error "The first argument of the 'colon' macro must be a string!"
%else
    %ifnid %2
        %error "The second argument of the 'colon' macro must be a label!"
    %else
        %2:
            dq first
            db %1, 0
        %define first %2
    %endif
%endif
%endmacro
