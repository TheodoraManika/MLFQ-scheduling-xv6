# MLFQ Scheduler Implementation για xv6

Εθνικό και Καποδιστριακό Πανεπιστήμιο Αθηνών  
Τμήμα Πληροφορικής και Τηλεπικοινωνιών  
Εργασία 2 - Λειτουργικά Συστήματα (K22)  
Ιανουάριος 2026

Μανίκα Θεοδώρα - 1115202100267

## Περιγραφή Εργασίας

Αυτή η εργασία υλοποιεί έναν MLFQ (Multilevel Feedback Queue) scheduler για το λειτουργικό σύστημα xv6. Ο νέος scheduler αντικαθιστά τον απλό round-robin scheduler με έναν πιο εξελιγμένο μηχανισμό που υποστηρίζει 4 επίπεδα προτεραιότητας.

## Αλλαγές στα Αρχεία

### 1. kernel/proc.h

**Προσθήκες:**
- Προσθήκη απαραίτητων includes στην αρχή του αρχείου:
  ```c
  #include "param.h"
  #include "types.h"
  #include "spinlock.h"
  #include "vm.h"
  ```

- Επέκταση της `struct proc` με πεδία για το MLFQ scheduling:
  ```c
  int priority;      // MLFQ priority level (0-3)
  int ticks_used;    // Ticks used in current queue
  int wait_count;    // Wait count for priority boost
  ```

**Αιτιολόγηση:**
- `priority`: Αποθηκεύει το τρέχον επίπεδο προτεραιότητας της διεργασίας (0 = υψηλότερη, 3 = χαμηλότερη)
- `ticks_used`: Μετράει πόσα timer ticks έχει χρησιμοποιήσει η διεργασία στο τρέχον επίπεδο προτεραιότητας
- `wait_count`: Μετράει πόσες φορές η διεργασία έχει περιμένει, για τον μηχανισμό αποφυγής starvation

### 2. kernel/proc.c

**Αλλαγές στην `allocproc()`:**
- Αρχικοποίηση των νέων πεδίων κατά τη δημιουργία νέας διεργασίας:
  ```c
  p->priority = 0;      // Νέες διεργασίες ξεκινούν στην υψηλότερη προτεραιότητα
  p->ticks_used = 0;
  p->wait_count = 0;
  ```

**Αλλαγές στην `freeproc()`:**
- Επαναφορά των πεδίων MLFQ όταν μια διεργασία τερματίζεται:
  ```c
  p->priority = 0;
  p->ticks_used = 0;
  p->wait_count = 0;
  ```

**Υλοποίηση MLFQ Scheduler στην `scheduler()`:**
- Ο scheduler τώρα αναζητά διεργασίες με βάση την προτεραιότητα
- Υλοποίηση 4 επιπέδων προτεραιότητας (0-3)
- Round-robin scheduling εντός κάθε επιπέδου προτεραιότητας
- Χρονομερίδια ανά επίπεδο:
  - Priority 0: 4 ticks
  - Priority 1: 8 ticks
  - Priority 2: 16 ticks
  - Priority 3: 32 ticks

**Μηχανισμός Demotion:**
- Όταν μια διεργασία καταναλώσει το χρονομερίδιο της, μειώνεται η προτεραιότητά της (αν δεν είναι ήδη στο επίπεδο 3)

**Μηχανισμός Priority Boost (Anti-Starvation):**
- Αν μια διεργασία περιμένει 10 φορές το χρονομερίδιό της χωρίς να εκτελεστεί, αυξάνεται η προτεραιότητά της

### 3. kernel/syscall.h

**Προσθήκη νέας κλήσης συστήματος:**
```c
#define SYS_getpinfo 22
```

### 4. kernel/syscall.c

**Προσθήκη:**
- Εξωτερική δήλωση της συνάρτησης: `extern uint64 sys_getpinfo(void);`
- Προσθήκη στον πίνακα syscalls: `[SYS_getpinfo] sys_getpinfo,`

### 5. kernel/sysproc.c

**Υλοποίηση `sys_getpinfo()`:**
- Αντιγραφή πληροφοριών από κάθε ενεργή διεργασία στη δομή `pstat`
- Πληροφορίες που αντιγράφονται:
  - PID (Process ID)
  - PPID (Parent Process ID)
  - Priority level
  - Process state
  - Process name
  - Size (memory size)

### 6. user/user.h

**Προσθήκη:**
- Δήλωση της νέας system call: `int getpinfo(struct pstat*);`
- Ορισμός της δομής `pstat`:
  ```c
  #define NPROC_STAT 64
  
  struct pstat {
    int pid;
    int ppid;
    int priority;
    int state;
    char name[16];
    uint64 sz;
  };
  ```

### 7. user/usys.pl

**Προσθήκη:**
```perl
entry("getpinfo");
```

Αυτό δημιουργεί το assembly stub για την κλήση συστήματος.

### 8. user/ps.c

**Νέο αρχείο - Υλοποίηση προγράμματος ps:**
- Καλεί την `getpinfo()` για να λάβει πληροφορίες για όλες τις ενεργές διεργασίες
- Εμφανίζει τις πληροφορίες σε μορφή πίνακα:
  - PID
  - PPID
  - Priority
  - State
  - Name

## Μεθοδολογία Υλοποίησης

### Βήμα 1: Προσθήκη Πεδίων στη struct proc
Πρώτα προσθέσαμε τα απαραίτητα πεδία στη δομή `proc` για να αποθηκεύουμε:
- Το επίπεδο προτεραιότητας
- Τα ticks που έχει χρησιμοποιήσει η διεργασία
- Τον αριθμό φορών που έχει περιμένει (για priority boost)

### Βήμα 2: Υλοποίηση Κλήσης Συστήματος getpinfo()
Ακολουθήθηκε η τυπική διαδικασία προσθήκης system call στο xv6:
1. Ορισμός του αριθμού της system call στο `syscall.h`
2. Προσθήκη του handler στο `syscall.c`
3. Υλοποίηση της λογικής στο `sysproc.c`
4. Δήλωση στο user space (`user.h`)
5. Δημιουργία stub (`usys.pl`)

### Βήμα 3: Τροποποίηση του Scheduler
Αντικαταστήσαμε τον απλό round-robin scheduler με MLFQ:
- Ο scheduler τώρα προτεραιοποιεί διεργασίες με βάση το επίπεδο προτεραιότητάς τους
- Εντός κάθε επιπέδου, εφαρμόζεται round-robin
- Υλοποίηση μηχανισμού demotion όταν εξαντλείται το χρονομερίδιο
- Υλοποίηση μηχανισμού priority boost για αποφυγή starvation

### Βήμα 4: Υλοποίηση Προγράμματος ps
Δημιουργήσαμε το user program `ps.c` που:
- Χρησιμοποιεί την `getpinfo()` για λήψη πληροφοριών
- Εμφανίζει τις πληροφορίες με ευανάγνωστο τρόπο

## Κανόνες MLFQ που Υλοποιήθηκαν

✅ 4 επίπεδα προτεραιότητας (0-3)  
✅ Υψηλότερη προτεραιότητα εκτελείται πρώτη  
✅ Round-robin εντός κάθε επιπέδου  
✅ Timer tick accounting  
✅ Χρονομερίδια: 4, 8, 16, 32 ticks  
✅ Νέες διεργασίες ξεκινούν στο επίπεδο 0  
✅ Demotion μετά από εξάντληση χρονομεριδίου  
✅ Διατήρηση υπόλοιπου χρονομεριδίου σε εθελοντική παράδοση CPU  
✅ Priority boost μετά από 10 αναμονές (anti-starvation)  
