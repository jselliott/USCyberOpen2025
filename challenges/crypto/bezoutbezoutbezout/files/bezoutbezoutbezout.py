for i in range(len(gcds)):
    d = gcds[i]
    a,b = magic_select(d, nums)
    s,t = magic_bezout(a,b)
    assert(d + s + t == ord(flag[i]))