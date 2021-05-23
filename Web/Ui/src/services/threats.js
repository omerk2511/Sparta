import axios from 'axios';

export default class ThreatsService {
    static async getAllThreats() {
        // TODO:: move to config
        const res = await axios.get('http://localhost:3000/api/threats');
        const threats = res.data.map(ThreatsService.formatThreat);

        return threats;
    }

    static async getThreat(id) {
        // TODO:: move to config
        const res = await axios.get(`http://localhost:3000/api/threats/${id}`);
        const threat = ThreatsService.formatThreat(res.data);

        return threat;
    }

    static formatThreat(threat) {
        return {
            ...threat,
            type: ThreatsService.formatThreatType(threat.type),
        }
    }

    static formatThreatType(type) {
        const formattedTypes = {
            'systemTokenStealing': { name: 'SYSTEM Token Stealing', path: '/arsenal/system-token-stealing' },
            'dllInjection': { name: 'DLL Injection', path: '/arsenal/dll-injection' },
            'apcInjection': { name: 'APC Injection', path: '/arsenal/apc-injection' },
            'remoteThreadCreation': { name: 'Remote Thread Creation', path: '/arsenal/remote-thread-creation' },
            'smepBypass': { name: 'SMEP Bypass', path: '/arsenal/smep-bypass' },
            'kernelStructureCorruption': { name: 'Kernel Structure Corruption', path: '/arsenal/kernel-structure-corruption' },
        };

        return formattedTypes[type];
    }
}
