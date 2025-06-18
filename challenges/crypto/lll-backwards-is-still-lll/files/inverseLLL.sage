I = identity_matrix(ZZ, 6)
M = Matrix(ZZ, 6, 6, map(ord, flag))
M.transpose().LLL()
#output:
[-12 -28   2  -2   0   9]
[  2 -19  -5  -2 -47  -2]
[ 43 -17  10  19  22  -1]
[ 24 -18  -3 -42  15 -19]
[  3  -7   6  18   5 -65]
[-24   2  74 -18 -21 -26]