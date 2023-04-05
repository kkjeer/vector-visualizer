#ifndef VECTORFIELD_EXPR
#define VECTORFIELD_EXPR
#include <string>
#include <math.h>
#include <iomanip>
#include <sstream>

using namespace std;

namespace Expression {
  enum ExprKind {
    ValKind,
    TKind,
    XKind,
    YKind,
    ZKind,
    NegKind,
    AddKind,
    SubKind,
    MultKind,
    DivKind,
    PowKind,
    SinKind,
    CosKind,
    LogKind
  };

  extern string Precision(float f, int precision = 2);
  
  class Expr {
  public:
    ExprKind Kind;
    virtual bool IncludeParens() { return false; }
    virtual string ToString() = 0;
    virtual float Eval(float x, float y, float z) = 0;
    virtual bool NumericValue(float &) { return false; }
    virtual bool IsZero() { return false; }
    virtual bool IsOne() { return false; }
    virtual Expr *Derivative(char wrt) = 0;
    virtual Expr *Simplify() = 0;
    string Wrap() {
      string str = ToString();
      if (IncludeParens()) return "(" + str + ")";
      return str;
    }
  };

  extern Expr *CreateCos(Expr *Child);
  
  class Val : public Expr {
  public:
    float V;
    int P;
    Val(float val, int precision = 0) : V(val), P(precision) {
      Kind = ExprKind::ValKind;
    }
  
    string ToString() {
      return Precision(V, P);
    }
    float Eval(float, float, float) {
      return V;
    }

    bool NumericValue(float &v) {
      v = V;
      return true;
    }
    bool IsZero() {
      return (int)V == 0;
    }
    bool IsOne() {
      return (int)V == 1;
    }

    // Constant rule: dc/dx = 0
    Expr *Derivative(char) {
      return new Val(0, 0);
    }

    Expr *Simplify() {
      return this;
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::ValKind;
    }
  };

  class T : public Expr {
  public:
    T() {
      Kind = ExprKind::TKind;
    }
    string ToString() {
      return "<i>t</i>";
    }
    float Eval(float x, float, float) {
      return x;
    }

    // Variable rule: dt/dt = 1, dt/dv = 0 where v != t
    Expr *Derivative(char wrt) {
      if (wrt == 't') {
        return new Val(1, 0);
      }
      return new Val(0, 0);
    }

    Expr *Simplify() {
      return this;
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::TKind;
    }
  };
  
  class X : public Expr {
  public:
    X() {
      Kind = ExprKind::XKind;
    }
    string ToString() {
      return "<i>x</i>";
    }
    float Eval(float x, float, float) {
      return x;
    }

    // Variable rule: dx/dx = 1, dx/dv = 0 where v != x
    Expr *Derivative(char wrt) {
      if (wrt == 'x') {
        return new Val(1, 0);
      }
      return new Val(0, 0);
    }

    Expr *Simplify() {
      return this;
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::XKind;
    }
  };

  class Y : public Expr {
  public:
    Y() {
      Kind = ExprKind::YKind;
    }
    string ToString() {
      return "<i>y</i>";
    }
    float Eval(float, float y, float) {
      return y;
    }

    // Variable rule: dy/dy = 1, dy/dv = 0 where y != t
    Expr *Derivative(char wrt) {
      if (wrt == 'y') {
        return new Val(1, 0);
      }
      return new Val(0, 0);
    }

    Expr *Simplify() {
      return this;
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::YKind;
    }
  };
  
  class Z : public Expr {
  public:
    Z() {
      Kind = ExprKind::ZKind;
    }
    string ToString() {
      return "<i>z</i>";
    }
    float Eval(float, float, float z) {
      return z;
    }

    // Variable rule: dz/dz = 1, dz/dv = 0 where z != t
    Expr *Derivative(char wrt) {
      if (wrt == 'z') {
        return new Val(1, 0);
      }
      return new Val(0, 0);
    }

    Expr *Simplify() {
      return this;
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::ZKind;
    }
  };

  class Neg : public Expr {
  public:
    Expr *Child;
    Neg(Expr *C) : Child(C) {
      Kind = ExprKind::NegKind;
    }
    string ToString() {
      return "-" + Child->Wrap();
    }
    float Eval(float x, float y, float z) {
      return -1 * Child->Eval(x, y, z);
    }

    bool NumericValue(float &v) {
      bool result = Child->NumericValue(v);
      v = -v;
      return result;
    }

    // Negation rule: (-f)' = -(f')
    Expr *Derivative(char wrt) {
      Expr *SubExpr = Child->Derivative(wrt);
      return new Neg(SubExpr);
    }

    Expr *Simplify() {
      Expr *SubExpr = Child->Simplify();
      float subVal;
      if (SubExpr->NumericValue(subVal)) {
        return new Val(-subVal, 2);
      }
      if (SubExpr->Kind == ExprKind::NegKind) {
        // --e == e
        return SubExpr;
      }
      return new Neg(SubExpr);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::NegKind;
    }
  };
  
  class Add : public Expr {
  public:
    Expr *Left;
    Expr *Right;
    Add(Expr *L, Expr *R) : Left(L), Right(R) {
      Kind = ExprKind::AddKind;
    }
  
    bool IncludeParens() { return true; }
    string ToString() {
      return Left->Wrap() + " + " + Right->Wrap();
    }
    float Eval(float x, float y, float z) {
      return Left->Eval(x, y, z) + Right->Eval(x, y, z);
    }

    // Addition rule: (f + g)' = f' + g'
    Expr *Derivative(char wrt) {
      Expr *L = Left->Derivative(wrt);
      Expr *R = Right->Derivative(wrt);
      return new Add(L, R);
    }

    Expr *Simplify() {
      Expr *L = Left->Simplify();
      Expr *R = Right->Simplify();
      if (L->Kind == ExprKind::ValKind && R->Kind == ExprKind::ValKind) {
        // Simplify addition of constants into a single constant
        Val *V1 = static_cast<Val *>(L);
        Val *V2 = static_cast<Val *>(R);
        float value = V1->V + V2->V;
        int precision = 0;
        if (V1->P > precision) {
          precision = V1->P;
        }
        if (V2->P > precision) {
          precision = V2->P;
        }
        return new Val(value, precision);
      } else if (L->IsZero()) {
        // 0 + e == e
        return R;
      } else if (R->IsZero()) {
        // e + 0 == e
        return L;
      }
      return new Add(L, R);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::AddKind;
    }
  };
  
  class Sub : public Expr {
  public:
    Expr *Left;
    Expr *Right;
    Sub(Expr *L, Expr *R) : Left(L), Right(R) {
      Kind = ExprKind::SubKind;
    }
  
    bool IncludeParens() { return true; }
    string ToString() {
      return Left->Wrap() + " - " + Right->Wrap();
    }
    float Eval(float x, float y, float z) {
      return Left->Eval(x, y, z) - Right->Eval(x, y, z);
    }

    // Subtraction rule: (f - g)' = f' - g'
    Expr *Derivative(char wrt) {
      Expr *L = Left->Derivative(wrt);
      Expr *R = Right->Derivative(wrt);
      return new Sub(L, R);
    }

    Expr *Simplify() {
      Expr *L = Left->Simplify();
      Expr *R = Right->Simplify();
      if (L->Kind == ExprKind::ValKind && R->Kind == ExprKind::ValKind) {
        // Simplify subtraction of constants into a single constant
        Val *V1 = static_cast<Val *>(L);
        Val *V2 = static_cast<Val *>(R);
        float value = V1->V - V2->V;
        int precision = 0;
        if (V1->P > precision) {
          precision = V1->P;
        }
        if (V2->P > precision) {
          precision = V2->P;
        }
        return new Val(value, precision);
      } else if (L->IsZero()) {
        // 0 - e = -e
        return (new Neg(R))->Simplify();
      } else if (R->IsZero()) {
        // e - 0 == e
        return L;
      } else if (R->Kind == ExprKind::NegKind) {
        // e1 - -e2 = e1 + e2
        Neg *N2 = static_cast<Neg *>(R);
        return (new Add(L, N2->Child))->Simplify();
      }
      return new Sub(L, R);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::SubKind;
    }
  };
  
  class Mult : public Expr {
  public:
    Expr *Left;
    Expr *Right;
    Mult(Expr *L, Expr *R) : Left(L), Right(R) {
      Kind = ExprKind::MultKind;
    }
  
    bool IncludeParens() { return true; }
    string ToString() {
      return Left->Wrap() + " * " + Right->Wrap();
    }
    float Eval(float x, float y, float z) {
      return Left->Eval(x, y, z) * Right->Eval(x, y, z);
    }

    // Product rule: (f * g)' = fg' + gf'
    Expr *Derivative(char wrt) {
      Expr *Fprime = Left->Derivative(wrt);
      Expr *Gprime = Right->Derivative(wrt);
      return new Add(new Mult(Left, Gprime), new Mult(Right, Fprime));
    }

    Expr *Simplify() {
      Expr *L = Left->Simplify();
      Expr *R = Right->Simplify();
      if (L->Kind == ExprKind::ValKind && R->Kind == ExprKind::ValKind) {
        // Simplify multiplication of constants into a single constant
        Val *V1 = static_cast<Val *>(L);
        Val *V2 = static_cast<Val *>(R);
        float value = V1->V * V2->V;
        int precision = 0;
        if (V1->P > precision) {
          precision = V1->P;
        }
        if (V2->P > precision) {
          precision = V2->P;
        }
        return new Val(value, precision);
      } else if (L->IsZero()) {
        // 0 * e == 0
        return new Val(0, 0);
      } else if (R->IsZero()) {
        // e * 0 == 0
        return new Val(0, 0);
      } else if (L->IsOne()) {
        // 1 * e == e
        return R;
      } else if (R->IsOne()) {
        return L;
      }
      return new Mult(L, R);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::MultKind;
    }
  };
  
  class Div : public Expr {
  public:
    Expr *Left;
    Expr *Right;
    Div(Expr *L, Expr *R) : Left(L), Right(R) {
      Kind = ExprKind::DivKind;
    }
  
    bool IncludeParens() { return true; }
    string ToString() {
      return Left->Wrap() + " / " + Right->Wrap();
    }
    float Eval(float x, float y, float z) {
      float r = Right->Eval(x, y, z);
      if (r <= 0.00001) {
        return 1000000000.0;
      }
      return Left->Eval(x, y, z) / r;
    }

    // Quotient rule: (f / g)' = (gf' - fg')/g^2
    Expr *Derivative(char wrt) {
      Expr *Fprime = Left->Derivative(wrt);
      Expr *Gprime = Right->Derivative(wrt);
      Expr *Gsquared = new Mult(Right, Right);
      Expr *Numerator = new Sub(new Mult(Right, Fprime), new Mult(Left, Gprime));
      return new Div(Numerator, Gsquared);
    }

    Expr *Simplify() {
      Expr *L = Left->Simplify();
      Expr *R = Right->Simplify();
      if (L->Kind == ExprKind::ValKind && R->Kind == ExprKind::ValKind) {
        // Simplify multiplication of constants into a single constant
        Val *V1 = static_cast<Val *>(L);
        Val *V2 = static_cast<Val *>(R);
        float value = 1000000;
        if ((int)V2->V != 0)
          value = V1->V / V2->V;
        int precision = 0;
        if (V1->P > precision) {
          precision = V1->P;
        }
        if (V2->P > precision) {
          precision = V2->P;
        }
        return new Val(value, precision);
      } else if (R->IsOne()) {
        // e/1 == e
        return L;
      } else if (L->IsZero()) {
        // 0/e = 0
        return new Val(0, 0);
      }
      return new Div(L, R);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::DivKind;
    }
  };

  class Log : public Expr {
  public:
    Expr *Child;
    Log(Expr *C) : Child(C) {
      Kind = ExprKind::LogKind;
    }
  
    string ToString() {
      return "ln(" + Child->ToString() + ")";
    }
    float Eval(float x, float y, float z) {
      return log(Child->Eval(x, y, z));
    }

    // Natural log rule: ln(x)' = 1/x
    Expr *Derivative(char ) {
      return new Div(new Val(1, 0), Child);
    }

    Expr *Simplify() {
      Expr *SubExpr = Child->Simplify();
      return new Log(SubExpr);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::LogKind;
    }
  };
  
  class Pow : public Expr {
  public:
    Expr *Left;
    Expr *Right;
    Pow(Expr *L, Expr *R) : Left(L), Right(R) {
      Kind = ExprKind::PowKind;
    }
  
    string ToString() {
      return Left->Wrap() + "<sup>" + Right->Wrap() + "</sup>";
    }
    float Eval(float x, float y, float z) {
      return pow(Left->Eval(x, y, z), Right->Eval(x, y, z));
    }

    // Power rule: (f ^ g)' = (f^g) * (g'*ln(f) + g*f'/f)
    Expr *Derivative(char wrt) {
      Expr *Fprime = Left->Derivative(wrt);
      Expr *Gprime = Right->Derivative(wrt);
      Expr *GprimeLnF = new Mult(Gprime, new Log(Left));
      Expr *GFprimeF = new Div(new Mult(Right, Fprime), Left);
      return new Mult(new Pow(Left, Right), new Add(GprimeLnF, GFprimeF));
    }

    Expr *Simplify() {
      Expr *L = Left->Simplify();
      Expr *R = Right->Simplify();
      if (L->Kind == ExprKind::ValKind && R->Kind == ExprKind::ValKind) {
        // TODO: simplify exponentiation of constants
      }
      return new Pow(L, R);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::PowKind;
    }
  };
  
  class Sin : public Expr {
  public:
    Expr *Child;
    Sin(Expr *C) : Child(C) {
      Kind = ExprKind::SinKind;
    }
  
    string ToString() {
      return "sin(" + Child->ToString() + ")";
    }
    float Eval(float x, float y, float z) {
      return sin(Child->Eval(x, y, z));
    }

    // Sine rule: sin(x)' = cos(x)
    Expr *Derivative(char ) {
      return CreateCos(Child);
    }

    Expr *Simplify() {
      Expr *SubExpr = Child->Simplify();
      return new Sin(SubExpr);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::SinKind;
    }
  };

  class Cos : public Expr {
  public:
    Expr *Child;
    Cos(Expr *C) : Child(C) {
      Kind = ExprKind::CosKind;
    }
  
    string ToString() {
      return "cos(" + Child->ToString() + ")";
    }
    float Eval(float x, float y, float z) {
      return cos(Child->Eval(x, y, z));
    }

    // Cosine rule: cos(x)' = -sin(x)
    Expr *Derivative(char ) {
      return new Neg(new Sin(Child));
    }

    Expr *Simplify() {
      Expr *SubExpr = Child->Simplify();
      return new Cos(SubExpr);
    }

    static bool classof(Expr *E) {
      return E->Kind == ExprKind::CosKind;
    }
  };
}

#endif
