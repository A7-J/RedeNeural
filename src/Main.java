public class Main {
    public static void main(String[] args) {
        Neuronio n1 = new Neuronio("A", 0);
        Neuronio n2 = new Neuronio("B", 1);
        Neuronio n3 = new Neuronio("C", 2);

        n1.conectar(n2);
        n2.conectar(n3);

        MotorGrafico motor = new MotorGrafico();
        Neuronio.setMotor(motor);

        // estimula em thread separada pra não travar a janela
        new Thread(() -> {
            try {
                Thread.sleep(1000);
                n1.receberEstimulo(0.4);
                Thread.sleep(500);
                n1.receberEstimulo(0.4); // A dispara
                Thread.sleep(500);
                n2.receberEstimulo(0.4); // B dispara
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }).start();

        // abre a janela (bloqueia aqui)
        motor.inicializar(600, 400);
    }
}