C
C     An example to calculate the area and angles of a triangle
C     given the lengths of the sides.
C
      PROGRAM TRIANG
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      LOGICAL FLAG
      PRINT *, ' Enter the lengths of the sides of the triangle '
      READ *, A, B, C
      PRINT *, ' The lengths of the sides of the triangle are ', A, B, C
      FLAG = (A + B) .GT. C .AND. (B + C) .GT. A .AND. (C + A) .GT. B
      IF (FLAG) THEN
        S = 0.5 * (A + B + C)
        AREA = SQRT(S * (S - A) * (S - B) * (S - C))
        ANGA = ANGVAL(B, C, A)
        ANGB = ANGVAL(C, A, B)
        ANGC = 180 - (ANGA + ANGB)
        PRINT *, ' The area of triangle is ', AREA
        PRINT *, ' The angles of triangle are ', ANGA, ANGB, ANGC
      ELSE
        PRINT *, ' Invalid input data'
      END IF
      END
      FUNCTION ANGVAL(X, Y, Z)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      PI = 3.14159265D0
      TEMP  = (X * X + Y * Y - Z * Z) / (2 * X * Y)
      ANGVAL = (180 / PI) * ACOS(TEMP)
      END
