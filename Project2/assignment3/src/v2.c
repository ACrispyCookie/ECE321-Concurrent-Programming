void *car(void *args) {
    car_info_t *info = (car_info_t *) args;
    int i = info->team;
    
    printf("%ld: Start\n", info->id);
    mysem_down(&q[i]);
    mysem_down(&mtx[i]);
    printf("%ld: Down team\n", info->id); 
    // accessing bridge
    if (cars[i] == 0) {
        printf("%ld: Was first\n", info->id);
        waiting[i] = 1;
        mysem_down(&bridge_access);
        waiting[i] = 0;
        printf("%ld: Got first\n", info->id);
    }
    cars[i]++;
    total_cars[i]++;
    
    mysem_down(&wait_decision);
    if (/*total cars not needed*/total_cars[i] < N && cars[i] < N && waiting[!i]) {
        printf("%ld: Up next %d\n", info->id, from_non_waiting);
        mysem_up(&q[i]);
    } else if (cars[i] < N && !waiting[!i]) {
        printf("%ld: Up next, %d\n", info->id, from_non_waiting);
        mysem_up(&q[i]);
    }
    from_non_waiting = !waiting[!i];
    printf("%ld: from_non_waiting: %d\n", info->id, from_non_waiting);
    mysem_up(&wait_decision);

    printf("%ld: Up team\n", info->id);
    mysem_up(&mtx[i]);

    printf("%ld: Crossing\n", info->id);
    sleep(3);
    printf("%ld: Done crossing\n", info->id);

    mysem_down(&mtx[i]);
    printf("%ld: Down team leaving\n", info->id);
    cars[i]--;
    if (total_cars[i] >= N && cars[i] == N - 1 /*this means it was full until I left and it cur thinks it's full still cause i havent opened mtx yet*/ && from_non_waiting && cars[i]) {
        mysem_up(&q[i]);
    } else if (total_cars[i] >= N && cars[i] == N - 1 /*this means it was full until I left and it cur thinks it's full still cause i havent opened mtx yet*/ && from_non_waiting && !cars[i]) {
        mysem_up(&q[i]);
        total_cars[i] = 0;
        from_non_waiting = false;
        mysem_up(&bridge_access);
    } else if (cars[i] == 0) {
        printf("%ld: Was last, %d\n", info->id, from_non_waiting);
        if (total_cars[i] >= N && !from_non_waiting) {
            printf("%ld: Wake first\n", info->id);
            mysem_up(&q[i]);
        }
        total_cars[i] = 0;
        from_non_waiting = false;
        mysem_up(&bridge_access);
    }
    printf("%ld: Up team leaving\n", info->id);
    mysem_up(&mtx[i]);

    return NULL;
}