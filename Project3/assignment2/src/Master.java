import java.util.*;

public class Master {

    private int waiting;
    private final Queue<WorkerData> data;
    private final List<Worker> workers;

    public Master(int workerCount) {
        data = new LinkedList<>();
        // Create N worker threads
        workers = new ArrayList<>();
        for (int i = 0; i < workerCount; i++) {
            Worker worker = new Worker(this);
            workers.add(worker);
            worker.start();
        }
    }

    public void start() {
        // Read value to give to workers
        Scanner scanner = new Scanner(System.in);
        while (scanner.hasNext()) {
            System.out.println("Master: Adding new data to queue!");
            add(new WorkerData(Command.PROCESS, scanner.nextInt()));
        }

        // Add N terminate commands to queue
        for (int i = 0; i < workers.size(); i++) {
            System.out.println("Master: Adding terminate data to queue!");
            add(new WorkerData(Command.TERMINATE, -1));
        }

        // Wait for workers to terminate
        try {
            for (Worker worker : workers)
                worker.join();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    public synchronized WorkerData get() {
        try {
            // Wait if another worker is waiting or there is no data available
            if (data.isEmpty() || waiting > 0) {
                waiting++;
                    wait();
                waiting--;
            }
            return data.poll();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    public synchronized void add(WorkerData data) {
        this.data.add(data);
        if (waiting > 0) {
            notify();
        }
    }
}
