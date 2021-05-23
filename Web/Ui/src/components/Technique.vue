<template>
    <v-main>
        <v-container class="py-8 px-6" fluid>
            <v-row>
                <v-col cols="12">
                    <v-card class="mx-auto">
                        <v-card-title><v-icon color="secondary" left>{{ technique.icon }}</v-icon> {{ technique.name }}</v-card-title>
                        <v-card-subtitle>{{ technique.description }}</v-card-subtitle>

                        <v-card-text>
                            <div class="text--primary mb-3">
                                {{ technique.explanation }}
                            </div>

                            <div class="text--primary mb-3">
                                Sample code:
                            </div>

                            <code style="display: block; white-space: pre-wrap; color: #ffffff; background: #000000;" class="pa-3">{{ technique.code }}</code>
                        </v-card-text>
                    </v-card>
                </v-col>
            </v-row>
        </v-container>
    </v-main>
</template>

<script>
import TechniquesService from '@/services/techniques';

export default {
    name: 'Technique',

    title: 'Sparta - Arsenal',

    data: () => ({
        technique: {}
    }),

    mounted() {
        const id = this.$route.params.id;

        this.technique = TechniquesService.getTechnique(id);
    },
}
</script>
