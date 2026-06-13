public class Main {

    static Neuronio a, b, c, d, e, f;

    public static void estimular(int id) {
        new Thread(() -> {
            try {
                switch (id) {
                    case 0 -> a.receberEstimulo(0.4);
                    case 1 -> b.receberEstimulo(0.4);
                    case 2 -> c.receberEstimulo(0.4);
                    case 3 -> d.receberEstimulo(0.4);
                    case 4 -> e.receberEstimulo(0.4);
                    case 5 -> f.receberEstimulo(0.4);
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }).start();
    }

    public static void main(String[] args) {
        a = new Neuronio("A", 0);
        b = new Neuronio("B", 1);
        c = new Neuronio("C", 2);
        d = new Neuronio("D", 3);
        e = new Neuronio("E", 4);
        f = new Neuronio("F", 5);

        a.conectar(c); a.conectar(d);
        b.conectar(c); b.conectar(d);
        c.conectar(e); c.conectar(f);
        d.conectar(e); d.conectar(f);

        MotorGrafico motor = new MotorGrafico();
        Neuronio.setMotor(motor);

        motor.setCallbackEstimulo(new Main());
        motor.inicializar(700, 500);
        motor.inicializar(700, 500);
    }
}