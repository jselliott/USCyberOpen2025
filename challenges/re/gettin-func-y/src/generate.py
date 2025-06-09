from random import shuffle

FLAG = "SVUSCG{1m_4_l1ttl3_t00_func_y_s0m3t1m3s}"

output = open("main.go","w")

output.write("import fmt")

idxs = [i for i in range(len(FLAG))]
shuffle(idxs)

for i in idxs:
    code = '''
func flag_%d() int {
    return %d
}
''' % (i,ord(FLAG[i]))

    output.write(code)

code = '''
func main(){
    fmt.Println("Where'd my flag go?")
}
'''

output.write(code)