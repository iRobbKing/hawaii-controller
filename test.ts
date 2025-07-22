// number { -1, 0, 1.2, NaN, ... }
// string { "a", "aa", "aaa", "-1" }
// boolean { true, false }
// null { null }

type User = {
    id: string
    age: number
}

type MyUser = [id: string, age: number]

// User { (-1, "a"), (-1, "aa"), ..., (0, "a"), ... }

type Result = "success" | "failure" | "idle"

type A = "a"

// A { a }

// Result { success, failure, idle }

// never {}

const handleResult = (result: Result): number => {
    if (result === "success") return 0;
    if (result === "failure") return 1;
    if (result === "idle") return 2;

    const buffer: Base64URLString = absurd(result);
    buffer.charAt(1);
};

const absurd = <T>(from: never): T => {
    throw 'asdfasdf';
}

type Either<TOk, TErr> = { ok: true, value: TOk } | { ok: false, value: TErr }

const fetchUser = (): Either<User, never> => {
    return { ok: true, value: { id: "", age: 12 } }
}

const user = fetchUser();

const canUserEnterClub = (user: Either<User, never>): boolean => {
    if (user.ok) return 18 <= user.value.age;

    return absurd(user.value);
};





const z = 42;
const w = z > 42 ? 1 : 2;

const y = (t) => t + 2


type A = number;
type B = A extends number | string ? symbol : object;

type Nullable<T> = T | null

type MyFunction = 
