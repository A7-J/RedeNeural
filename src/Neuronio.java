import java.util.ArrayList;
import java.util.List;

public class Neuronio {
    private String id;
    private double potencial;
    private List<Neuronio> conexoes;
    private static final double LIMIAR = 0.7;

    public Neuronio(String id) {
        this.id = id;
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
        for (Neuronio n : conexoes)
            n.receberEstimulo(potencial * 0.5);
    }
}