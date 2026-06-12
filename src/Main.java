public class Main {
    public static void main(String[] args) {
        // Camada de entrada
        Neuronio a = new Neuronio("A", 0);
        Neuronio b = new Neuronio("B", 1);

        // Camada oculta
        Neuronio c = new Neuronio("C", 2);
        Neuronio d = new Neuronio("D", 3);

        // Camada de saída
        Neuronio e = new Neuronio("E", 4);
        Neuronio f = new Neuronio("F", 5);

        // conexões entrada → oculta
        a.conectar(c);
        a.conectar(d);
        b.conectar(c);
        b.conectar(d);

        // conexões oculta → saída
        c.conectar(e);
        c.conectar(f);
        d.conectar(e);
        d.conectar(f);

        MotorGrafico motor = new MotorGrafico();
        Neuronio.setMotor(motor);

        new Thread(() -> {
            try {
                Thread.sleep(1000);
                a.receberEstimulo(0.4);
                Thread.sleep(300);
                a.receberEstimulo(0.4); // A dispara
                Thread.sleep(600);
                b.receberEstimulo(0.4);
                Thread.sleep(300);
                b.receberEstimulo(0.4); // B dispara
            } catch (InterruptedException ex) {
                ex.printStackTrace();
            }
        }).start();

        motor.inicializar(700, 500);
    }
}