-- Script pour mettre à jour les avatars des comptes de test
-- À exécuter avec: sqlite3 coinche.db < update_test_avatars.sql

UPDATE users SET avatar = 'avataaars1.svg' WHERE email = 'aaa@aaa.fr';
UPDATE users SET avatar = 'avataaars2.svg' WHERE email = 'bbb@bbb.fr';
UPDATE users SET avatar = 'avataaars3.svg' WHERE email = 'ccc@ccc.fr';
UPDATE users SET avatar = 'avataaars4.svg' WHERE email = 'ddd@ddd.fr';

-- Vérifier les mises à jour
SELECT pseudo, email, avatar FROM users WHERE email IN ('aaa@aaa.fr', 'bbb@bbb.fr', 'ccc@ccc.fr', 'ddd@ddd.fr');
