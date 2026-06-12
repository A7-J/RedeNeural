import java.util.ArrayList;
import java.util.List;

public class Neuronio {
    private String id;
    private int index; // 0=A, 1=B, 2=C
    private double potencial;
    private List<Neuronio> conexoes;
    private static final double LIMIAR = 0.7;
    private static MotorGrafico motor;

    public static void setMotor(MotorGrafico m) {
        motor = m;
    }

    public Neuronio(String id, int index) {
        this.id = id;
        this.index = index;
        this.potencial = 0.0;
        this.conexoes = new ArrayList<>();
    }

    public void conectar(Neuronio outro) {
        this.conexoes.add(outro);
    }

    public void receberEstimulo(double valor) {
        this.potencial += valor;
        System.out.println(id + " | potencial: " + potencial);
        if (this.potencial >= LIMIAR) {
            dispararAcao();
            this.potencial = 0.0;
        }
    }

    private void dispararAcao() {
        System.out.println(">>> " + id + " DISPAROU!");
        if (motor != null) {
            motor.atualizarNeuronio(index, true);
            try { Thread.sleep(400); } catch (InterruptedException e) {}
            motor.atualizarNeuronio(index, false);
        }
        for (Neuronio n : conexoes)
            n.receberEstimulo(potencial * 0.5);
    }
}