if __name__ == '__main__':
    import sys
    maxid = 0
    codes = {}
    for line in sys.stdin:
        line = line.strip()
        label = line[0]
        if label == '-':
            label = '-1'
        flds = line.split('|')
        reason = flds[3][1:]
        reasons = reason.split()
        new_reasons = []
        for r in reasons:
            s_code,val = r.split(':')
            if not val:continue
            codeid = codes.get(s_code)
            if codeid == None:
                codeid = maxid
                maxid += 1
                codes[s_code] = codeid
            new_reasons.append('%s:%s'%(codeid, val))
        if new_reasons:
            print '%s %s'%(label, ' '.join(new_reasons))
        
