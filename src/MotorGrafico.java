public class MotorGrafico {
    static {
        System.loadLibrary("motor_grafico");
    }

    public native void inicializar(int largura, int altura);
    public native void atualizarNeuronio(int id, boolean ativo);
    public native void atualizarContador(int id, int valor);
    public native void setCallbackEstimulo(Object mainClass);
}