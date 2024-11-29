public class Worker extends Thread {
    private final Master master;

    public Worker(Master master) {
        this.master = master;
    }

    public void run() {
        while(true) {
            WorkerData data;
            data = master.get();
            if (data.getCommand() == Command.TERMINATE) {
                System.out.println("Worker #" + getId() + ": Terminating...");
                return;
            }
            System.out.println("Worker #" + getId() + ": " + data.getValue() + " is " + (isPrime(data.getValue()) ? "" : "not ") + "prime");
        }
    }

    private boolean isPrime(int N) {
        for (long i = 2; i < N; i++) {
            if (N % i == 0)
                return false;
        }
        return true;
    }
}
