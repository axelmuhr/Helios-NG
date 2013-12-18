IMPLEMENTATION MODULE Termbase;
  (* Copyright (C) 1989, Rowley Associates
                         32 Rowley
                         Cam
                         Dursley
                         Glos. GL11 5NT
                         England.

     Version  :  1.00  21-May-89  Paul Curtis
                   Initial release for Rowley Modula-2 under Helios.
  *)

  FROM SYSTEM IMPORT
    ADR;
  FROM POSIX IMPORT
    fdstream, write, read;
  FROM SysLib IMPORT
    StreamPtr, Flags_MSdos;
  FROM ASCII IMPORT
    EOL, CR, LF;
  FROM Strings IMPORT
    Length;
  FROM CharClass IMPORT
    IsCntrl;
  IMPORT
    SysLib;

  VAR
    bufch: CHAR;

  PROCEDURE ReadString(VAR str: ARRAY OF CHAR; VAR done: BOOLEAN);
    VAR i: CARDINAL;
        ch: CHAR;

    PROCEDURE AppendCh(ch: CHAR);
    BEGIN
      IF i <= HIGH(str) THEN
        str[i] := ch; INC(i);
      END
    END AppendCh;

  BEGIN
    i := 0;
    LOOP
      Read(ch, done);
      IF NOT done OR IsCntrl(ch) THEN EXIT
      ELSE AppendCh(ch)
      END
    END;
    IF i <= HIGH(str) THEN str[i] := 0C END
  END ReadString;

  PROCEDURE ReadLine(VAR str: ARRAY OF CHAR; VAR done: BOOLEAN);
    VAR i: CARDINAL;
        ch: CHAR;

    PROCEDURE AppendCh(ch: CHAR);
    BEGIN
      IF i <= HIGH(str) THEN
        str[i] := ch; INC(i);
      END
    END AppendCh;

  BEGIN
    i := 0;
    LOOP
      Read(ch, done);
      IF NOT done OR (ch = EOL) THEN EXIT
      ELSE AppendCh(ch)
      END
    END;
    IF i <= HIGH(str) THEN str[i] := 0C END
  END ReadLine;

  PROCEDURE Read(VAR ch: CHAR; VAR done: BOOLEAN);
    VAR err: INTEGER;
  BEGIN
    REPEAT UNTIL KeyPressed();
    IF bufch = LF THEN bufch := EOL END;
    ch := bufch; done := TRUE
  END Read;

  PROCEDURE KeyPressed(): BOOLEAN;
    VAR strm: StreamPtr;
        read: CARDINAL;
  BEGIN
    strm := fdstream(0);
    read := SysLib.Read(strm^, ADR(bufch), 1, 1000);
    RETURN read > 0
  END KeyPressed;

  PROCEDURE Write(ch: CHAR);
    VAR s: ARRAY [0..9] OF CHAR;
        strm: StreamPtr;
  BEGIN
    IF ch = EOL THEN
      strm := fdstream(1);
      IF BITSET(Flags_MSdos) * BITSET(strm^.Flags) # {} THEN
        s[0] := CR; s[1] := LF; s[2] := 0C;
      ELSE
        s[0] := LF; s[1] := 0C
      END
    ELSE s[0] := ch; s[1] := 0C
    END;
    WriteString(s)
  END Write;

  (*$V-*)
  PROCEDURE WriteString(s: ARRAY OF CHAR);
    VAR err: INTEGER;
  BEGIN
    err := write(1, ADR(s), Length(s));
  END WriteString;
  (*$V=*)

END Termbase.
