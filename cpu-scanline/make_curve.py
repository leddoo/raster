import sys

if len(sys.argv) == 4:
    print(f"curve( " +
        f"  x({sys.argv[1]})*(1-t)^2 " +
        f"+ 2x({sys.argv[2]})*(1-t)*t " +
        f"+ x({sys.argv[3]})*t^2, " +
        f"  y({sys.argv[1]})*(1-t)^2 " +
        f"+ 2y({sys.argv[2]})*(1-t)*t " +
        f"+ y({sys.argv[3]})*t^2, " +
        "t, 0, 1)"
    )
elif len(sys.argv) == 5:
    print(f"curve( " +
        f"  x({sys.argv[1]})*(1-t)^3 " +
        f"+ 3x({sys.argv[2]})*(1-t)^(2)*t " +
        f"+ 3x({sys.argv[3]})*(1-t)*t^2 " +
        f"+ x({sys.argv[4]})*t^3, " +
        f"  y({sys.argv[1]})*(1-t)^3 " +
        f"+ 3y({sys.argv[2]})*(1-t)^(2)*t " +
        f"+ 3y({sys.argv[3]})*(1-t)*t^2 " +
        f"+ y({sys.argv[4]})*t^3, " +
        "t, 0, 1)"
    )
else:
    assert False
