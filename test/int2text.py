def int_to_text(n: int) -> tuple[int, ...]:

    if n < 0:
        raise ValueError('Negative')
    elif n < 20:
        return (n,)
    elif n < 100:
        return (n // 10 * 10, n % 10,)
    elif n < 1000 and n % 100 == 0:
        return (n,)
    elif n < 1000 and n % 100 != 0 and n % 10 == 0:
        return (n // 100 * 100, n % 100,)
    elif n < 1000 and n % 100 != 0 and n % 10 != 0:
        return (n // 100 * 100, n % 100 // 10 * 10, n % 10,)
    else:
        raise ValueError('Large')
