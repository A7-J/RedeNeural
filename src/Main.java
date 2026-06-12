public class Main {
    public static void main(String[] args) {
        Neuronio n1 = new Neuronio("A");
        Neuronio n2 = new Neuronio("B");
        Neuronio n3 = new Neuronio("C");

        n1.conectar(n2);
        n2.conectar(n3);

        n1.receberEstimulo(0.4);
        n1.receberEstimulo(0.4); // deve disparar aqui

        n2.receberEstimulo(0.4); // não dispara
    }
}